#include "code_editor_view.h"
#include "code_editor_manager.h"
#include "textdocument.h"

#include "core/coreconstants.h"
#include "codeassist/documentcontentcompletion.h"
//#include "codeassist/keywordscompletionassist.h"
#include "qmljsindenter.h"
#include "qmljsautocompleter.h"
#include <QScrollBar>
#include <QTimer>
#include <QDebug>
namespace ady{
class CodeEditorViewPrivate{
public:
    int line=0;
    int column=0;
    TextEditor::DocumentContentCompletionProvider* provider;
};

CodeEditorView::CodeEditorView(QWidget* parent)
    :TextEditor::TextEditorWidget(parent)
{
    d = new CodeEditorViewPrivate;
    d->provider = new TextEditor::DocumentContentCompletionProvider;
    this->setAutoCompleter(new QmlJSEditor::AutoCompleter);

    auto doc = QSharedPointer<TextEditor::TextDocument>(new TextEditor::TextDocument(Core::Constants::K_DEFAULT_TEXT_EDITOR_ID));
    this->setTextDocument(doc);
    //auto keyWords = TextEditor::Keywords({"while","if"});
    //static TextEditor::DocumentContentCompletionProvider basicSnippetProvider;

    doc->setCompletionAssistProvider(d->provider);
    //doc->completionAssistProvider();
    //auto indenter = new TextEditor::TextIndenter(doc->document());
    auto indenter = new QmlJSEditor::Internal::Indenter(doc->document());
    doc->setIndenter(indenter);
}

CodeEditorView::~CodeEditorView(){
    delete d->provider;
    delete d;
}

void CodeEditorView::contextMenuEvent(QContextMenuEvent *e){
    auto instance = CodeEditorManager::getInstance();
    if(instance!=nullptr){
        QMenu contextMenu(this);
        instance->editorContextMenu(this,&contextMenu);
        contextMenu.exec(QCursor::pos());
    }else{
        TextEditor::TextEditorWidget::contextMenuEvent(e);
    }
}

void CodeEditorView::showEvent(QShowEvent *e){
    TextEditor::TextEditorWidget::showEvent(e);
}

void CodeEditorView::rename(const QString& name){
    this->textDocument()->setFilePath(Utils::FilePath::fromString(name));
}

/*void CodeEditorView::delayGoto(int line,int column){
    if(this->isHidden()==false){
        d->line = line;
        d->column = column;
    }else{

    }
    this->gotoLine(line,column);

}*/


}
