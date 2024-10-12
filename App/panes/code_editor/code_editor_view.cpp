#include "code_editor_view.h"
#include "code_editor_manager.h"
#include "textdocument.h"

#include "core/coreconstants.h"
#include "codeassist/documentcontentcompletion.h"
//#include "codeassist/keywordscompletionassist.h"
//#include "languages/cstyle/cstyleindenter.h"
//#include "languages/cstyle/cstyleautocomplete.h"
//#include "languages/python/pythonindenter.h"
#include "languages/html/htmlindenter.h"
#include "languages/html/htmlautocompleter.h"

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
    auto indenter = Html::createIndenter(doc->document());
    doc->setIndenter(indenter);
    this->setAutoCompleter(new Html::AutoCompleter);
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

    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QsciLexerCPP *lexer = new QsciLexerCPP;
    this->setLexer(lexer);


    /****************line no style start*******************/
    //this->setCaretLineVisible(false);
    this->setMarginType(0, QsciScintilla::NumberMargin);
    this->setMarginWidth(0, 50);
    this->setMarginWidth(10, "00000");
    this->setMarginsForegroundColor(Qt::black);
    this->setMarginsBackgroundColor(Qt::lightGray);

    QFont font;
    font.setFamily("Courier");
    font.setPointSize(10);
    this->setMarginsFont(font);
    /****************line no style end*******************/


    /****************code fold start*******************/
    this->setFolding(QsciScintilla::BoxedTreeFoldStyle);
    this->setMarginType(2, QsciScintilla::SymbolMargin);
    this->setMarginWidth(2, 12);
    this->setMarginSensitivity(2, true);
    this->setFoldMarginColors(Qt::lightGray,Qt::lightGray);
    /****************code fold end*******************/
    this->setMarginType(1, QsciScintilla::TextMargin);


    this->setAutoCompletionSource(QsciScintilla::AcsAll);
    this->setAutoCompletionThreshold(2);

    this->setAutoIndent(true);
    this->setTabWidth(4);
    this->setIndentationGuides(true);

    this->setBraceMatching(QsciScintilla::SloppyBraceMatch);

    this->setWrapMode(QsciScintilla::WrapNone);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    this->setScrollWidth(5);
    this->setScrollWidthTracking(true);

    //this->setWrapMode(QsciScintilla::WrapWord);

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

void CodeEditorView::resizeEvent(QResizeEvent * e){
     QsciScintilla::resizeEvent(e);
     qDebug()<<"maximumViewportSize:"<<maximumViewportSize();
}

void CodeEditorView::rename(const QString& name){
    //this->textDocument()->setFilePath(Utils::FilePath::fromString(name));
     //d->path = name;
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

