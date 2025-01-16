#include "code_editor_view.h"
#include "code_editor_manager.h"
#include <textdocument.h>
#include <core/coreconstants.h>
#include <codeassist/documentcontentcompletion.h>
#include <syntaxhighlighter.h>
#include <semantichighlighter.h>
#include <textmark.h>
#include <texteditorsettings.h>
#include <fontsettings.h>

#include <QApplication>
#include <QClipboard>
#include <QScrollBar>
#include <QTimer>
#include <QAction>
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
    auto doc = QSharedPointer<TextEditor::TextDocument>(new TextEditor::TextDocument(Core::Constants::K_DEFAULT_TEXT_EDITOR_ID));
    this->setTextDocument(doc);
    doc->setCompletionAssistProvider(d->provider);
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




void CodeEditorView::addSemanticError(int line,int column,int length,const QString& message){
    //this->clearSemanticErrorMarks();
        if(line<1){
            return ;
        }
        if(length==0){
            auto doc = this->document();
            if(column>0){
                column -= 1;
            }
            length = doc->findBlockByLineNumber(line - 1).length() - column;

        }
        auto textDocument = this->textDocument();
        auto mark = new TextEditor::TextMark(textDocument->filePath(),line,TextEditor::Constants::SEMANTIC_ERROR_ID);
        mark->setIcon(QIcon(":/Resource/icons/StatusCriticalError_16x.svg"));
        mark->setColor(Utils::Theme::CodeModel_Error_TextMarkColor);
        mark->setDefaultToolTip(tr("Code Semantic Error"));
        mark->setToolTip(message);
        mark->setPriority(TextEditor::TextMark::HighPriority);
        mark->setLineAnnotation(message);

        mark->setActionsProvider([this,message](){
            QList<QAction*> actions;
            auto action = new QAction(QIcon(":/Resource/icons/Copy_16x.svg"),tr("Copy"),this);
            actions<<action;
            connect(action,&QAction::triggered,[this,message](){
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(message);
            });
            return actions;
        });

        textDocument->addMark(mark);
        auto highlighter = textDocument->syntaxHighlighter();
        //int line, int column, int length, int kind
        TextEditor::HighlightingResult hlr(line,column,length,TextEditor::C_ERROR);
        QHash<int, QTextCharFormat> kindToFormat;
        QList<TextEditor::HighlightingResult> results;
        results<<hlr;
        QTextCharFormat format = TextEditor::TextEditorSettings::fontSettings().toTextCharFormat(TextEditor::C_ERROR);
        kindToFormat.insert(TextEditor::C_ERROR,format);
        TextEditor::SemanticHighlighter::setExtraAdditionalFormats(highlighter,results,kindToFormat);
}

void CodeEditorView::clearSemanticError(){
    auto textDocument = this->textDocument();
    auto highlighter = textDocument->syntaxHighlighter();
    TextEditor::SemanticHighlighter::setExtraAdditionalFormats(highlighter,{},{});

     //remove error marks
    this->clearSemanticErrorMarks();
}

void CodeEditorView::clearSemanticErrorMarks(){
    auto textDocument = this->textDocument();
    auto marks = textDocument->marks();
    for(auto mark:marks){
        if(mark->category()==TextEditor::Constants::SEMANTIC_ERROR_ID){
            textDocument->removeMark(mark);
        }
    }
}



}

