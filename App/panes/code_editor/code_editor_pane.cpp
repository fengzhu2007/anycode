#include "code_editor_pane.h"
#include "code_editor.h"
#include "code_editor_manager.h"
#include "docking_pane_container.h"
#include "docking_pane_container_tabbar.h"
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
#include <QDebug>

namespace ady{

const QString CodeEditorPane::PANE_ID = "CodeEditor_%1";
const QString CodeEditorPane::PANE_GROUP = "CodeEditor";
int CodeEditorPane::SN = 0;

class CodeEditorPanePrivate{
public:
    CodeEditor* editor;
    int id;
};

CodeEditorPane::CodeEditorPane(QWidget *parent)
    :DockingPane(parent)
{
    //Subscriber::reg();
    d = new CodeEditorPanePrivate;
    d->id = CodeEditorPane::SN;
    d->editor = new CodeEditor(this);
    this->setCenterWidget(d->editor);
    CodeEditorManager::getInstance()->append(this);
    CodeEditorPane::SN += 1;
    connect(d->editor,&QPlainTextEdit::modificationChanged,this,&CodeEditorPane::onModificationChanged);
}


CodeEditorPane::~CodeEditorPane(){
    //Subscriber::unReg();
    auto manager = CodeEditorManager::getInstance();
    if(manager!=nullptr){
        manager->remove(this);
    }
    delete d;
}

QString CodeEditorPane::id(){
    return PANE_ID.arg(d->id);
}

QString CodeEditorPane::group(){
    return PANE_GROUP;
}

QString CodeEditorPane::description(){
    return d->editor->path();
}

void CodeEditorPane::activation(){
    d->editor->init();
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
    if(d->editor->writeFile(path)){
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
    QMenu contextMenu(this);
    contextMenu.addAction(QIcon(":/Resource/icons/Save_16x.svg"),tr("Save(&S)"),this,[this](){
            this->save(false);
    },QKeySequence(Qt::CTRL + Qt::Key_S));
    contextMenu.addAction(tr("Close(&C)"),this,[this](){
            //this->close();
            auto container = this->container();
            container->closePane(this);
    },QKeySequence(Qt::CTRL + Qt::Key_F4));
    contextMenu.addAction(tr("Close Other(&A)"),this,[this](){
        auto container = this->container();
        //int i = container->current();
        int index = container->indexOf(this);
        int count  = container->paneCount();
        int pos = 0;
        for(int i=0;i<count;i++){
            if(i==index){
                pos += 1;
            }else{
                if(!container->closePane(pos)){
                    pos += 1;
                }
            }
        }
    });
    contextMenu.addAction(tr("Close All(&L)"),this,[this](){
        auto container = this->container();
        int count  = container->paneCount();
        int pos = 0;
        for(int i=0;i<count;i++){
            if(!container->closePane(pos)){
                pos += 1;
            }
        }
    });
    contextMenu.addSeparator();
    contextMenu.addAction(QIcon(":/Resource/icons/Copy_16x.svg"),tr("Copy Path(&U)"),this,[this](){
        QApplication::clipboard()->setText(d->editor->path());
    });
    contextMenu.addAction(tr("Open Folder(&O)"),this,[this](){
        QDesktopServices::openUrl(QFileInfo(d->editor->path()).absoluteDir().absolutePath());
    });
    contextMenu.addSeparator();
    contextMenu.addAction(tr("Float Tab(&F)"),this,[this](){
        auto container = this->container();
        int index = container->indexOf(this);
        if(index>=0)
            container->floatPane(index);
    });
    /*contextMenu.addAction(tr("Fixed Tab(&P)"),this,[this](){
        auto container = this->container();
    });*/
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
    this->setToolTip(path);
    return d->editor->readFile(path);
}

bool CodeEditorPane::writeFile(const QString& path){
    return d->editor->writeFile(path);
}

QString CodeEditorPane::path(){
    return d->editor->path();
}

bool CodeEditorPane::isModified() const{
    return d->editor->document()->isModified();
}

CodeEditor* CodeEditorPane::editor(){
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
