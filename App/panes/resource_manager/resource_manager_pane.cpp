#include "resource_manager_pane.h"
#include "ui_resource_manager_pane.h"
#include "resource_manager_model.h"
#include "resource_manager_model_item.h"
#include "resource_manager_tree_item_delegate.h"
#include "resource_manage_read_folder_task.h"
#include "core/event_bus/event.h"
#include "core/event_bus/publisher.h"
#include "core/backend_thread.h"
#include "storage/ProjectStorage.h"
#include "docking_pane_manager.h"
#include "docking_pane_layout_item_info.h"
#include "components/message_dialog.h"
#include "panes/code_editor/code_editor_manager.h"
#include "common/utils.h"
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QDesktopServices>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QMimeData>
#include <QDebug>



const QString mimeStr = "application/x-qt-windows-mime;value=\"Preferred DropEffect\"";

namespace ady{

ResourceManagerPane* ResourceManagerPane::instance = nullptr;

const QString ResourceManagerPane::PANE_ID = "ResourceManager";
const QString ResourceManagerPane::PANE_GROUP = "ResourceManager";


const QString ResourceManagerPane::M_OPEN_PROJECT = "Open_Message";
const QString ResourceManagerPane::M_FILE_CHANGED = "File_Changed";
const QString ResourceManagerPane::M_OPEN_EDITOR = "Open_Editor";


class ResourceManagerPanePrivate {
public:
    ResourceManagerModel* model;
    //FolderReader* reader;
    QAction* actionNew_Folder;
    QAction* actionNew_File;
    QAction* actionOpen_Folder;
    QAction* actionOpen_File;
    QAction* actionClose_Project;
    QAction* actionFind;
    QAction* actionOpen_Terminal;
    QAction* actionCopy;
    QAction* actionCut;
    QAction* actionPaste;
    QAction* actionRename;
    QAction* actionDelete;
    QAction* actionCopy_Path;
};

ResourceManagerPane::ResourceManagerPane(QWidget *parent) :
    DockingPane(parent),
    ui(new Ui::ResourceManagerPane)
{
    Subscriber::reg();
    this->regMessageIds({M_OPEN_PROJECT,M_FILE_CHANGED});
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(tr("Resource Manager"));
    this->setStyleSheet("QToolBar{border:0px;}"
                        "QTreeView{border:0;background-color:#f5f5f5}"
                        ".ady--ResourceManagerPane>QWidget{background-color:#EEEEF2}");

    ui->treeView->setItemDelegate(new ResourceManagerTreeItemDelegate(ui->treeView));
    ui->treeView->setEditTriggers(QAbstractItemView::EditKeyPressed);

    //init view
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView,&QTreeView::customContextMenuRequested, this, &ResourceManagerPane::showContextMenu);
    connect(ui->treeView,&QTreeView::expanded,this,&ResourceManagerPane::onTreeItemExpanded);
    connect(ui->treeView,&QTreeView::doubleClicked,this,&ResourceManagerPane::onTreeItemDClicked);


    d = new ResourceManagerPanePrivate;
    d->model = ResourceManagerModel::getInstance();
    ui->treeView->setModel(d->model);

    this->initView();
}

ResourceManagerPane::~ResourceManagerPane()
{
    instance = nullptr;
    Subscriber::unReg();
    delete ui;
    delete d;
}

void ResourceManagerPane::initView(){
    d->actionNew_Folder = new QAction(QIcon(":/Resource/icons/NewFileCollection_16x.svg"),tr("New Folder"),this);
    d->actionNew_File = new QAction(QIcon(":/Resource/icons/NewFile_16x.svg"),tr("New File"),this);
    d->actionOpen_Folder = new QAction(QIcon(":/Resource/icons/OpenFolder_16x.svg"),tr("Open Folder"),this);
    d->actionOpen_File = new QAction(QIcon(":/Resource/icons/OpenFile_16x.svg"),tr("Open File"),this);
    d->actionClose_Project = new QAction(tr("Close Project"),this);
    d->actionFind = new QAction(QIcon(":/Resource/icons/SearchFolderClosed_16x.svg"),tr("Find In Folder"),this);
    d->actionOpen_Terminal = new QAction(QIcon(":/Resource/icons/ImmediateWindow_16x.svg"),tr("Open Terminal"),this);
    d->actionCopy = new QAction(QIcon(":/Resource/icons/Copy_16x.svg"),tr("Copy"),this);
    d->actionCut = new QAction(QIcon(":/Resource/icons/Cut_16x.svg"),tr("Cut"),this);
    d->actionPaste = new QAction(QIcon(":/Resource/icons/Paste_16x.svg"),tr("Paste"),this);
    d->actionRename = new QAction(tr("Rename"),this);
    d->actionDelete = new QAction(QIcon(":/Resource/icons/Cancel_16x.svg"),tr("Delete"),this);
    d->actionCopy_Path = new QAction(tr("Copy Path"),this);
    d->actionCut->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_X));
    d->actionCopy->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_C));
    d->actionPaste->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_V));
    d->actionRename->setShortcut(QKeySequence(Qt::Key_F2));
    d->actionFind->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_F));

    connect(d->actionNew_Folder,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionNew_File,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionOpen_Folder,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionOpen_File,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionClose_Project,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionFind,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionOpen_Terminal,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionCopy,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionPaste,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionRename,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionDelete,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionCopy_Path,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionCut,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);



}

QString ResourceManagerPane::id(){
    return PANE_ID;
}

QString ResourceManagerPane::group(){
    return PANE_GROUP;
}

bool ResourceManagerPane::onReceive(Event* e){
    if(e->id()==M_OPEN_PROJECT){
        auto one = static_cast<ProjectRecord*>(e->data());
        auto project = new ProjectRecord();
        project->id = one->id;
        project->path = one->path;
        project->name = one->name;
        auto item = d->model->appendItem(project);
        this->readFolder(item);
        item->setExpanded(true);
        e->ignore();
        return true;
    }
    return false;
}

void ResourceManagerPane::readFolder(ResourceManagerModelItem* item){
    auto thread = BackendThread::getInstance();
    //FolderReader* thread = FolderReader::getInstance();
    if(!thread->isRunning()){
        thread->start();
    }
    thread->appendTask(new ResourceManageReadFolderTask(d->model,item->path()));
}

void ResourceManagerPane::onTreeItemExpanded(const QModelIndex& index){
    if(index.isValid()){
        auto item = static_cast<ResourceManagerModelItem*>(index.internalPointer());
        if(item!=nullptr){
            if(item->type()==ResourceManagerModelItem::Project || item->type()==ResourceManagerModelItem::Folder){
                if(!item->expanded()){
                    //add to read thread
                    this->readFolder(item);
                    item->setExpanded(true);
                    d->model->appendWatchDirectory(item->path());
                }
            }

        }
    }
}

void ResourceManagerPane::onTreeItemDClicked(const QModelIndex& index){
    //qDebug()<<"onTreeItemDClicked";
    if(index.isValid()){
        auto item = static_cast<ResourceManagerModelItem*>(index.internalPointer());
        if(item->type()==ResourceManagerModelItem::File){
            //qDebug()<<"onTreeItemDClicked File";
            QString path = item->path();
            Publisher::getInstance()->post(M_OPEN_EDITOR,&path);
        }
    }
}

void ResourceManagerPane::showContextMenu(const QPoint& pos){
    QMenu contextMenu(this);
    QModelIndexList list = ui->treeView->selectionModel()->selectedRows();
    QModelIndex index = ui->treeView->indexAt(pos);
    if(!index.isValid()){
        return ;
    }
    auto one = static_cast<ResourceManagerModelItem*>(index.internalPointer());
    ResourceManagerModelItem::Type type = one->type();
    QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    QByteArray ba = mimeData->data(mimeStr);
    d->actionPaste->setEnabled(false);
    if(!ba.isEmpty()){
        char c = ba.at(0);
        if(c==5 || c==2){
            d->actionPaste->setEnabled(true);
        }
    }
    if(type==ResourceManagerModelItem::Project){
        contextMenu.addAction(d->actionNew_File);
        contextMenu.addAction(d->actionNew_Folder);
        contextMenu.addAction(d->actionOpen_Folder);
        contextMenu.addAction(d->actionOpen_Terminal);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionPaste);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionFind);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionClose_Project);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionCopy_Path);
    }else if(type==ResourceManagerModelItem::Folder){
        contextMenu.addAction(d->actionNew_File);
        contextMenu.addAction(d->actionNew_Folder);
        contextMenu.addAction(d->actionOpen_Folder);
        contextMenu.addAction(d->actionOpen_Terminal);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionCut);
        contextMenu.addAction(d->actionCopy);
        contextMenu.addAction(d->actionPaste);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionFind);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionRename);
        contextMenu.addAction(d->actionDelete);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionCopy_Path);
    }else if(type==ResourceManagerModelItem::File){
        contextMenu.addAction(d->actionOpen_File);
        contextMenu.addAction(d->actionOpen_Terminal);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionCut);
        contextMenu.addAction(d->actionCopy);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionRename);
        contextMenu.addAction(d->actionDelete);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionCopy_Path);
    }else{
        return ;
    }
    contextMenu.exec(QCursor::pos());
}

void ResourceManagerPane::onActionTriggered(){
    QObject* sender = this->sender();
    QModelIndexList list = ui->treeView->selectionModel()->selectedRows();
    if(list.size()<=0){
        return ;
    }
    auto one = static_cast<ResourceManagerModelItem*>(list.at(0).internalPointer());
    if(sender==d->actionNew_File){

    }else if(sender==d->actionNew_Folder){

    }else if(sender==d->actionOpen_Folder){
        QDesktopServices::openUrl(QFileInfo(one->path()).absoluteDir().absolutePath());
    }else if(sender==d->actionOpen_File){
        QString path = one->path();
        Publisher::getInstance()->post(M_OPEN_EDITOR,&path);
    }else if(sender==d->actionOpen_Terminal){

    }else if(sender==d->actionCut){
        QMimeData *mimeData = new QMimeData;
        QList<QUrl> urls;
        urls.append(QUrl::fromLocalFile(one->path()));
        mimeData->setUrls(urls);
        mimeData->setData(mimeStr, QByteArray("\x02\x00\x00\x00", 4));
        QApplication::clipboard()->setMimeData(mimeData);
    }else if(sender==d->actionCopy){
        QMimeData *mimeData = new QMimeData;
        QList<QUrl> urls;
        urls.append(QUrl::fromLocalFile(one->path()));
        mimeData->setUrls(urls);
        mimeData->setData(mimeStr, QByteArray("\x05\x00\x00\x00", 4));
        QApplication::clipboard()->setMimeData(mimeData);
    }else if(sender==d->actionPaste){
        const QMimeData *mimeData = QApplication::clipboard()->mimeData();
        if(mimeData->hasUrls()){
            QByteArray ba = mimeData->data(mimeStr);
            //QFileInfo fi(one->path());
            QString path = one->path();
            if(!path.endsWith("/")){
                path += "/";
            }
            if(!ba.isEmpty()){
                auto instance = Publisher::getInstance();
                char c = ba.at(0);
                QList<QUrl> urls = mimeData->urls();
                if(c==5){
                    //copy
                    foreach(auto one,urls){
                        QFileInfo fi(one.toLocalFile());
                        if(fi.exists()){
                            const QString fileName = fi.fileName();
                            QFile file(one.toLocalFile());
                            QString dest = path+fileName;
                            bool exists = false;
                            int i = 1;
                            //qDebug()<<"file:"<<one.toLocalFile()<<";"<<fi.isFile()<<";"<<fi.suffix();
                            do{
                                exists = QFileInfo::exists(dest);
                                if(exists){
                                    //dest = path + QString::fromUtf8("%1(%2)").arg(fileName).arg(i);
                                    if(fi.isFile() && fi.suffix().length()>0){
                                        dest = path + QString::fromUtf8("%1(%2).%3").arg(fi.baseName()).arg(i).arg(fi.suffix());
                                    }else{
                                        dest = path + QString::fromUtf8("%1(%2)").arg(fileName).arg(i);
                                    }
                                    i++;
                                }else{
                                    break;
                                }
                            }while(true);
                            if(fi.isFile()){
                                if(file.copy(dest)){
                                    //post update editor
                                    //instance->post(new Event(CodeEditorManager::M_RELOAD_FILE,(void*)&dest));
                                }
                            }else if(fi.isDir()){
                                if(Utils::copy(one.toLocalFile(),dest)){
                                    //post update editor
                                    //instance->post(new Event(CodeEditorManager::M_RELOAD_FILE,(void*)&dest));
                                }
                            }
                        }
                    }
                }else if(c==2){
                    //cut

                    foreach(auto one,urls){
                        QFileInfo fi(one.toLocalFile());
                        if(fi.exists()){
                            const QString oPath = fi.absoluteFilePath();
                            const QString fileName = fi.fileName();
                            //fi.baseName();

                            QString dest = path+fileName;
                            bool exists = false;
                            int i = 1;
                            do{
                                exists = QFileInfo::exists(dest);
                                if(exists){
                                    if(fi.isFile() && fi.suffix().length()>0){
                                        dest = path + QString::fromUtf8("%1(%2).%3").arg(fi.baseName()).arg(i).arg(fi.suffix());
                                    }else{
                                        dest = path + QString::fromUtf8("%1(%2)").arg(fileName).arg(i);
                                    }

                                    i++;
                                }else{
                                    break;
                                }
                            }while(true);
                            if(fi.isFile()){
                                QFile file(one.toLocalFile());
                                QPair<QString,QString> pair = {oPath,dest};
                                instance->post(new Event(CodeEditorManager::M_WILL_RENAME_FILE,(void*)&oPath));
                                if(!file.rename(dest)){
                                    pair.second = oPath;
                                }
                                instance->post(new Event(CodeEditorManager::M_RENAMED_FILE,(void*)&pair));
                            }else if(fi.isDir()){
                                instance->post(new Event(CodeEditorManager::M_WILL_RENAME_FOLDER,(void*)&oPath));
                                QDir dir(oPath);
                                QPair<QString,QString> pair = {oPath,dest};
                                if(!dir.rename(oPath,dest)){
                                    pair.second = oPath;
                                }
                                instance->post(new Event(CodeEditorManager::M_RENAMED_FOLDER,(void*)&pair));
                            }
                        }
                    }
                }
            }
        }

    }else if(sender==d->actionFind){

    }else if(sender==d->actionRename){
        ui->treeView->edit(list.at(0));
    }else if(sender==d->actionDelete){
        if(MessageDialog::confirm(this,tr("Delete Confirm"),tr("Are you sure you want to delete the '%1'?\nYou can restore this file from the Recycle Bin.").arg(one->title()),QMessageBox::Ok|QMessageBox::Cancel)==QMessageBox::Ok){
            auto instance = Publisher::getInstance();
            const QString path = one->path();
            QPair<QString,QString> pair = {path,path};
            ResourceManagerModelItem::Type type = one->type();
            if(type==ResourceManagerModelItem::File){
                instance->post(new Event(CodeEditorManager::M_WILL_RENAME_FILE,(void*)&path));
                if(QFile::moveToTrash(one->path())){
                    instance->post(new Event(CodeEditorManager::M_DELETE_FILE,(void*)&pair));
                }else{
                    instance->post(new Event(CodeEditorManager::M_RENAMED_FILE,(void*)&pair));
                }
            }else if(type==ResourceManagerModelItem::Folder){
                //remove watch
                QStringList list = d->model->takeWatchDirectory(path,true);
                instance->post(new Event(CodeEditorManager::M_WILL_RENAME_FOLDER,(void*)&path));
                if(QFile::moveToTrash(one->path())){
                    instance->post(new Event(CodeEditorManager::M_DELETE_FOLDER,(void*)&path));
                }else{
                    foreach(auto one,list){
                        d->model->appendWatchDirectory(one);
                    }
                    instance->post(new Event(CodeEditorManager::M_RENAMED_FOLDER,(void*)&pair));
                }
            }
        }
    }else if(sender==d->actionClose_Project){
        if(MessageDialog::confirm(this,tr("Close Confirm"),tr("Are you sure you want to close '%1'?").arg(one->title()),QMessageBox::Ok|QMessageBox::Cancel)==QMessageBox::Ok){
            d->model->removeItem(one);
        }
    }else if(sender==d->actionCopy_Path){
        QApplication::clipboard()->setText(one->path());
    }
}

ResourceManagerPane* ResourceManagerPane::open(DockingPaneManager* dockingManager,bool active){
    if(instance==nullptr){
        instance = new ResourceManagerPane(dockingManager->widget());
        DockingPaneLayoutItemInfo* item = dockingManager->createPane(instance,DockingPaneManager::S_Left,active);
        item->setStretch(300);
    }
    return instance;
}


}
