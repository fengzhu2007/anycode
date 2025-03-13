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

void AIClient::openDocument(TextEditor::TextDocument* document){
    connect(document,&TextEditor::TextDocument::contentsChangedWithPosition,this,[this, document](int position, int charsRemoved, int charsAdded) {
        Q_UNUSED(charsRemoved)
        if (!enabled())
            return;

        auto instance = CodeEditorManager::getInstance();
        auto pane = instance->current();
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
    });
}

void AIClient::scheduleRequest(CodeEditorView *editor){
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
    } else {
        it->cursorPosition = editor->textCursor().position();
    }
    it->timer->start(d->m_settings.m_triggerTimeout);
}

void AIClient::requestCompletions(CodeEditorView *editor)
{

    Utils::MultiTextCursor cursor = editor->multiTextCursor();
    if (cursor.hasMultipleCursors() || cursor.hasSelection() || editor->suggestionVisible())
        return;

    auto textCursor = cursor.mainCursor();
    int line = textCursor.blockNumber();
    int column = textCursor.columnNumber();

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
        //{"path",d->editor->p}
    };
    qDebug()<<"data"<<data;

    const Utils::FilePath filePath = editor->textDocument()->filePath();
    /*AiRequest request{{TextDocumentIdentifier(hostPathToServerUri(filePath)),documentVersion(filePath),Utils::Text::Position(cursor.mainCursor())}};
    request.setResponseCallback([this, editor = QPointer<CodeEditorView>(editor)](
                                    const GetCompletionRequest::Response &response) {
        QTC_ASSERT(editor, return);
        handleCompletions(response, editor);
    });
    m_runningRequests[editor] = request;*/
    //sendMessage(request);
}

void AIClient::handleCompletions(AIResponse *response,CodeEditorView *editor)
{

    if (!response->status())
        return ;

    int requestPosition = -1;
    /*if (const auto requestParams = m_runningRequests.take(editor).params())
        requestPosition = requestParams->position().toPositionInDocument(editor->document());

    const Utils::MultiTextCursor cursors = editor->multiTextCursor();
    if (cursors.hasMultipleCursors())
        return;

    if (cursors.hasSelection() || cursors.mainCursor().position() != requestPosition)
        return;


    if (const std::optional<GetCompletionResponse> result = response.result()) {
        auto isValidCompletion = [](const Completion &completion) {
            return completion.isValid() && !completion.text().trimmed().isEmpty();
        };
        QList<Completion> completions = Utils::filtered(result->completions().toListOrEmpty(),
                                                        isValidCompletion);

        // remove trailing whitespaces from the end of the completions
        for (Completion &completion : completions) {
            const LanguageServerProtocol::Range range = completion.range();
            if (range.start().line() != range.end().line())
                continue; // do not remove trailing whitespaces for multi-line replacements

            const QString completionText = completion.text();
            const int end = int(completionText.size()) - 1; // empty strings have been removed above
            int delta = 0;
            while (delta <= end && completionText[end - delta].isSpace())
                ++delta;

            if (delta > 0)
                completion.setText(completionText.chopped(delta));
        }
        auto suggestions = Utils::transform(completions, [](const Completion &c){
            auto toTextPos = [](const LanguageServerProtocol::Position pos){
                return Utils::Text::Position{pos.line() + 1, pos.character()};
            };

            Utils::Text::Range range{toTextPos(c.range().start()), toTextPos(c.range().end())};
            Utils::Text::Position pos{toTextPos(c.position())};
            return TextEditor::TextSuggestion::Data{range, pos, c.text()};
        });
        if (completions.isEmpty())
            return;
        editor->insertSuggestion(std::make_unique<TextEditor::CyclicSuggestion>(suggestions, editor->document()));
    }*/
}

void AIClient::cancelRunningRequest(CodeEditorView *editor)
{
    const auto it = m_runningRequests.constFind(editor);
    if (it == m_runningRequests.constEnd())
        return;
    //cancelRequest(it->id());
    m_runningRequests.erase(it);
}

bool AIClient::enabled(){
    return d->m_settings.m_enable && d->m_settings.m_apiKey.isEmpty()==false;
}


}
