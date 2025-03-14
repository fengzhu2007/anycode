#include "ai_client.h"
#include <textdocument.h>
#include <texteditor.h>
#include "modules/options/options_settings.h"
#include "modules/options/ai_settings.h"
#include "modules/ai/ai_response.h"
#include "code_editor_manager.h"
#include "code_editor_view.h"
#include <utils/multitextcursor.h>
#include <utils/textutils.h>
#include <utils/algorithm.h>
#include <textsuggestion.h>
#include <QPointer>

#include <QTimer>

namespace ady{
class AIClientPrivate{
public:
    explicit AIClientPrivate(const AISettings& settings):m_settings(settings){

    }
    AISettings m_settings;
};


AIClient::AIClient(QObject* parent):QObject(parent) {
    d = new AIClientPrivate(OptionsSettings::getInstance()->aiSettings());
}

AIClient::~AIClient(){
    for(auto one:m_runningRequests){
        one->setCallbackResponse(nullptr);
    }
    m_runningRequests.clear();
    delete d;
}

void AIClient::openDocument(TextEditor::TextDocument* document){
    connect(document,&TextEditor::TextDocument::contentsChangedWithPosition,this,[this, document](int position, int charsRemoved, int charsAdded) {
        Q_UNUSED(charsRemoved)
        if (!enabled())
            return;

        auto instance = CodeEditorManager::getInstance();
        auto pane = instance->current();
        if(pane){
            auto widget = pane->editor();
            if(widget==nullptr || widget->textDocument()!=document){
                return ;
            }
            if (widget->isReadOnly() || widget->multiTextCursor().hasMultipleCursors())
                return;
            const int cursorPosition = widget->textCursor().position();
            if (cursorPosition < position || cursorPosition > position + charsAdded)
                return;
            scheduleRequest(widget);
        }
    });
}

void AIClient::request(CodeEditorView* editor,int timeout){
    cancelRunningRequest(editor);
    auto it = m_scheduledRequests.find(editor);
    if (it == m_scheduledRequests.end()) {
        auto timer = new QTimer(this);
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, this, [this, editor]() {

            if (m_scheduledRequests[editor].cursorPosition == editor->textCursor().position())
                requestCompletions(editor);
        });
        connect(editor, &CodeEditorView::destroyed, this, [this, editor]() {
            delete m_scheduledRequests.take(editor).timer;
            cancelRunningRequest(editor);
        });
        connect(editor, &CodeEditorView::cursorPositionChanged, this, [this, editor] {
            cancelRunningRequest(editor);
        });
        it = m_scheduledRequests.insert(editor, {editor->textCursor().position(), timer});
    }else{
        it->cursorPosition = editor->textCursor().position();
    }
    it->timer->start(timeout);
}

void AIClient::scheduleRequest(CodeEditorView *editor){
    this->request(editor,d->m_settings.m_triggerTimeout);
}

void AIClient::requestCompletions(CodeEditorView *editor)
{
    Utils::MultiTextCursor cursor = editor->multiTextCursor();
    if (cursor.hasMultipleCursors() || cursor.hasSelection() || editor->suggestionVisible())
        return;
    auto textCursor = cursor.mainCursor();
    int line = textCursor.blockNumber();
    int column = textCursor.columnNumber();

    Utils::Text::Position position {line,column};

    auto document = editor->textDocument()->document();
    auto block = document->findBlockByNumber(line);
    auto text = document->toPlainText();
    auto nextBlock = block.next();

    auto previous = text.left(block.position());
    auto suffix = text.mid(nextBlock.position());
    auto middle = block.text();

    QJsonObject data = {
        {"prefix",previous},
        {"suffix",suffix},
        {"middle",middle.trimmed().isEmpty()?"":middle},
    };

    const Utils::FilePath filePath = editor->textDocument()->filePath();
    auto req = new AiRequest(nullptr,d->m_settings.m_apiKey,data);
    qDebug()<<"req"<<req;
    req->setCallbackResponse([this,position,editor = QPointer<CodeEditorView>(editor)](AIResponse* response){
        handleCompletions(response,position, editor);
    });
    connect(req,&AiRequest::finish,this,&AIClient::onRequestFinish);
    m_runningRequests[editor] = req;
    req->call();
}

void AIClient::handleCompletions(AIResponse *response,const Utils::Text::Position& position,CodeEditorView *editor)
{
    response->debug();
    if (response->status()){
        const Utils::MultiTextCursor cursors = editor->multiTextCursor();
        if (cursors.hasMultipleCursors()){
            goto result;
        }
        auto cursor = cursors.mainCursor();
        //qDebug()<<"result"<<(cursors.hasSelection())<<((cursor.blockNumber()==position.line && cursor.columnNumber()==position.column));
        //qDebug()<<"result"<<cursor.blockNumber()<<cursor.columnNumber();
        if (cursors.hasSelection() || (cursor.blockNumber()!=position.line || cursor.columnNumber()!=position.column)){
            goto result;
        }
        auto text = response->suggestion();
        if(!text.trimmed().isEmpty()){
            auto ret = cursor.block().text().trimmed().isEmpty();
            Utils::Text::Position start{position.line+1,ret?0:position.column};
            Utils::Text::Position end{start.line,text.length()};
            Utils::Text::Range range{start,end};
            QList<TextEditor::TextSuggestion::Data> suggestions;
            suggestions.append(TextEditor::TextSuggestion::Data{range,start,response->suggestion()});
            editor->insertSuggestion(std::make_unique<TextEditor::CyclicSuggestion>(suggestions, editor->document()));
        }
    }
    result:
    delete response;
    return ;
}

void AIClient::cancelRunningRequest(CodeEditorView *editor)
{
    const auto it = m_runningRequests.constFind(editor);
    if (it == m_runningRequests.constEnd())
        return;
    (*it)->setCallbackResponse(nullptr);
    m_runningRequests.erase(it);
}

bool AIClient::enabled(){
    return d->m_settings.m_enable && d->m_settings.m_apiKey.isEmpty()==false && d->m_settings.m_triggerPolicy==AISettings::Auto;
}

void AIClient::onRequestFinish(){
    auto sender = static_cast<AiRequest*>(this->sender());
    qDebug()<<"onRequestFinish"<<sender;
    auto iter = m_runningRequests.begin();
    while(iter!=m_runningRequests.end()){
        if(iter.value()==sender){
            m_runningRequests.erase(iter);
            break ;
        }
        iter++;
    }
    delete sender;
    //sender->deleteLater();
}


}
