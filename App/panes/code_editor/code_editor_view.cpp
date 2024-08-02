#include "code_editor_view.h"
#include "code_editor_manager.h"
#include "textdocument.h"
#include <QScrollBar>
#include <QTimer>
#include <QDebug>
namespace ady{
CodeEditorView::CodeEditorView(QWidget* parent)
    :TextEditor::TextEditorWidget(parent)
{

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

void CodeEditorView::rename(const QString& name){
    this->textDocument()->setFilePath(Utils::FilePath::fromString(name));
}



}
