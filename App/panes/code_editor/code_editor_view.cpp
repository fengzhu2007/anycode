#include "code_editor_view.h"
#include "code_editor_manager.h"
#include "textdocument.h"

#include "core/coreconstants.h"
#include "codeassist/documentcontentcompletion.h"
//#include "codeassist/keywordscompletionassist.h"
#include "languages/cstyle/cstyleindenter.h"
#include "languages/cstyle/cstyleautocomplete.h"
//#include "languages/python/pythonindenter.h"
#include <QScrollBar>
#include <QTimer>
#include <QDebug>
namespace ady{


#ifndef Scintilla


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


    auto doc = QSharedPointer<TextEditor::TextDocument>(new TextEditor::TextDocument(Core::Constants::K_DEFAULT_TEXT_EDITOR_ID));
    this->setTextDocument(doc);
    //auto keyWords = TextEditor::Keywords({"while","if"});
    //static TextEditor::DocumentContentCompletionProvider basicSnippetProvider;
    doc->setCompletionAssistProvider(d->provider);
    //doc->completionAssistProvider();
    //auto indenter = new TextEditor::TextIndenter(doc->document());
    //auto indenter = TextEditor::createCStyleIndenter(doc->document());
    //auto indenter = Python::createPythonIndenter(doc->document());
    //doc->setIndenter(indenter);
    this->setAutoCompleter(new CStyle::AutoCompleter);
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


#else

class CodeEditorViewPrivate{
public:
    int line=0;
    int column=0;
};

CodeEditorView::CodeEditorView(QWidget* parent)
    :QsciScintilla(parent)
{

    d = new CodeEditorViewPrivate;

}

CodeEditorView::~CodeEditorView(){
    delete d;
}

void CodeEditorView::contextMenuEvent(QContextMenuEvent *e){
    auto instance = CodeEditorManager::getInstance();
    if(instance!=nullptr){
        QMenu contextMenu(this);
        instance->editorContextMenu(this,&contextMenu);
        contextMenu.exec(QCursor::pos());
    }
}

void CodeEditorView::showEvent(QShowEvent *e){
    QsciScintilla::showEvent(e);
}

void CodeEditorView::rename(const QString& name){
    //this->textDocument()->setFilePath(Utils::FilePath::fromString(name));
}

void CodeEditorView::gotoLine(int line){

}

QTextCursor CodeEditorView::textCursor(){
    return {};
}

void CodeEditorView::findText(const QString& text,int flags,bool hightlight){

}

void CodeEditorView::replaceText(const QString&before,const QString& after,int flags,bool hightlight){

}

void CodeEditorView::clearHighlights(){

}

#endif


}
