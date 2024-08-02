#include "code_editor_pane.h"
//#include "code_editor.h"
#include "code_editor_manager.h"
#include "docking_pane_container.h"
#include "docking_pane_container_tabbar.h"


#include "core/coreconstants.h"
#include "textdocument.h"
#include "codeassist/documentcontentcompletion.h"
#include "textindenter.h"
#include "cppqtstyleindenter.h"
#include "javaindenter.h"

#include <QFileInfo>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QTextBlock>
#include <QPainter>
#include <QFileDialog>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QTextCodec>
#include <QDebug>

namespace ady{

const QString CodeEditorPane::PANE_ID = "CodeEditor_%1";
const QString CodeEditorPane::PANE_GROUP = "CodeEditor";
int CodeEditorPane::SN = 0;

class CodeEditorPanePrivate{
public:
    CodeEditorView* editor;
    int id;
};

CodeEditorPane::CodeEditorPane(QWidget *parent)
    :DockingPane(parent)
{
    //Subscriber::reg();
    d = new CodeEditorPanePrivate;
    d->id = CodeEditorPane::SN;
    d->editor = new CodeEditorView(this);
    this->setCenterWidget(d->editor);
    CodeEditorManager::getInstance()->append(this);
    CodeEditorPane::SN += 1;
    connect(d->editor,&QPlainTextEdit::modificationChanged,this,&CodeEditorPane::onModificationChanged);
}


CodeEditorPane::~CodeEditorPane(){
    //Subscriber::unReg();
    //qDebug()<<"CodeEditorPane::~CodeEditorPane";
    qDebug()<<"CodeEditorPane::~CodeEditorPane"<<this;
    auto manager = CodeEditorManager::getInstance();
    if(manager!=nullptr){
        manager->remove(this);
    }
    //disconnect(d->editor,&QPlainTextEdit::modificationChanged,this,&CodeEditorPane::onModificationChanged);
    delete d;
}

QString CodeEditorPane::id(){
    return PANE_ID.arg(d->id);
}

QString CodeEditorPane::group(){
    return PANE_GROUP;
}

QString CodeEditorPane::description(){
    return this->path();
}

void CodeEditorPane::activation(){
    //d->editor->init();
}

void CodeEditorPane::save(bool rename){
    QString path = this->path();
    bool tabRename = false;
    if(rename){
        //save as
        path = QFileDialog::getSaveFileName(this, tr("Save File AS"), "", tr("All Files (*.*)"));
        tabRename = true;
    }else if(path.isEmpty()){
        //new file save
        path = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("All Files (*.*)"));
        tabRename = true;
    }
    if (path.isEmpty()) {
        return ;
    }
    //save
    auto instance = CodeEditorManager::getInstance();
    instance->removeWatchFile(this->path());
    if(this->writeFile(path)){
        //rename tab title
        this->setToolTip(path);
        if(tabRename){
            QFileInfo fi(path);
            auto container = this->container();
            if(container!=nullptr){
                int i = container->indexOf(this);
                if(i>=0){
                    auto tabBar = container->tabBar();
                    tabBar->setTabText(i,fi.fileName());
                    tabBar->setTabToolTip(i,path);
                }
            }
        }
        d->editor->document()->setModified(false);
    }
    instance->appendWatchFile(path);

}

void CodeEditorPane::contextMenu(const QPoint& pos){
    QMenu contextMenu;
    auto instance = CodeEditorManager::getInstance();
    if(instance!=nullptr){
        instance->tabContextMenu(this,&contextMenu);
    }else{
        return ;
    }
    contextMenu.exec(QCursor::pos());
}

/*bool CodeEditorPane::onReceive(Event* e){

    return false;
}*/

void CodeEditorPane::rename(const QString& name){
    auto container = this->container();
    if(container!=nullptr){
        int i = container->indexOf(this);
        if(i>=0){
            QFileInfo fi(name);
            auto tabBar = container->tabBar();
            QString text = tabBar->tabText(i);
            tabBar->setTabText(i,fi.fileName());
            tabBar->setTabToolTip(i,name);
            this->editor()->rename(name);
        }
    }
}

bool CodeEditorPane::readFile(const QString& path){
    auto doc = d->editor->textDocumentPtr();
    if(!doc){
        doc = QSharedPointer<TextEditor::TextDocument>(new TextEditor::TextDocument(Core::Constants::K_DEFAULT_TEXT_EDITOR_ID));
        d->editor->setTextDocument(doc);
        static TextEditor::DocumentContentCompletionProvider basicSnippetProvider;
        doc->setCompletionAssistProvider(&basicSnippetProvider);
        //auto indenter = new TextEditor::TextIndenter(doc->document());
        auto indenter = new Android::Internal::JavaIndenter(doc->document());
        doc->setIndenter(indenter);
    }
    doc->setCodec(QTextCodec::codecForName("UTF-8"));
    QString error;
    auto ret = doc->open(&error,Utils::FilePath::fromString(path),Utils::FilePath::fromString(path));
    return true;
}

bool CodeEditorPane::writeFile(const QString& path){
    QString error;
    auto ret = d->editor->textDocument()->save(&error,Utils::FilePath::fromString(path),false);
    return true;
    //return d->editor->writeFile(path);
}

QString CodeEditorPane::path(){
    return d->editor->textDocument()->filePath().path();
}

bool CodeEditorPane::isModified() const{
    return d->editor->document()->isModified();
}

CodeEditorView* CodeEditorPane::editor(){
    return d->editor;
}

void CodeEditorPane::onModificationChanged(bool changed){
    auto container = this->container();
    if(container!=nullptr){
        int i = container->indexOf(this);
        if(i>=0){
            auto tabBar = container->tabBar();
            QString text = tabBar->tabText(i);
            if(changed){
                if(!text.endsWith("*")){
                    text +="*";
                }
            }else{
                if(text.endsWith("*")){
                    text = text.left(text.length() - 1);
                }
            }
            tabBar->setTabText(i,text);
        }
    }
}


}
