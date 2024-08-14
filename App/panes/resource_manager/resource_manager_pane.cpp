#include "resource_manager_pane.h"
#include "ui_resource_manager_pane.h"
#include "resource_manager_model.h"
#include "resource_manager_model_item.h"
#include "resource_manager_tree_item_delegate.h"
#include "resource_manage_read_folder_task.h"
#include "core/event_bus/event.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event_data.h"
#include "core/backend_thread.h"
#include "storage/project_storage.h"
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
#include <QProcess>
#include <QDebug>



const QString mime = "application/x-qt-windows-mime;value=\"Preferred DropEffect\"";

namespace ady{

ResourceManagerPane* ResourceManagerPane::instance = nullptr;

const QString ResourceManagerPane::PANE_ID = "ResourceManager";
const QString ResourceManagerPane::PANE_GROUP = "ResourceManager";




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
    QAction* actionUpload;
};

ResourceManagerPane::ResourceManagerPane(QWidget *parent) :
    DockingPane(parent),
    ui(new Ui::ResourceManagerPane)
{
    Subscriber::reg();
    this->regMessageIds({Type::M_OPEN_PROJECT,Type::M_FILE_CHANGED});
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(tr("Resource Manager"));
    this->setStyleSheet("QToolBar{border:0px;}"
                        "QTreeView{border:0;background-color:#f5f5f5}"
                        ".ady--ResourceManagerPane>#widget{background-color:#EEEEF2}");

    ui->treeView->setItemDelegate(new ResourceManagerTreeItemDelegate(ui->treeView));
    ui->treeView->setEditTriggers(QAbstractItemView::EditKeyPressed);

    //init view
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView,&QTreeView::customContextMenuRequested, this, &ResourceManagerPane::onContextMenu);
    connect(ui->treeView,&QTreeView::expanded,this,&ResourceManagerPane::onTreeItemExpanded);
    connect(ui->treeView,&QTreeView::doubleClicked,this,&ResourceManagerPane::onTreeItemDClicked);

    connect(ui->actionExpand,&QAction::triggered,this,&ResourceManagerPane::onTopActionTriggered);
    connect(ui->actionCollapse,&QAction::triggered,this,&ResourceManagerPane::onTopActionTriggered);


    d = new ResourceManagerPanePrivate;
    d->model = ResourceManagerModel::getInstance();
    ui->treeView->setModel(d->model);

    connect(d->model,&ResourceManagerModel::insertReady,this,&ResourceManagerPane::onInsertReady);

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
    d->actionUpload = new QAction(QIcon(":/Resource/icons/BatchCheckIn_16x.svg"),tr("Upload"),this);

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
    connect(d->actionUpload,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);


}

QString ResourceManagerPane::id(){
    return PANE_ID;
}

QString ResourceManagerPane::group(){
    return PANE_GROUP;
}

bool ResourceManagerPane::onReceive(Event* e){
    if(e->id()==Type::M_OPEN_PROJECT){
        auto one = static_cast<ProjectRecord*>(e->data());
        auto project = new ProjectRecord();
        project->id = one->id;
        project->path = one->path;
        project->name = one->name;
        qDebug()<<"pid:"<<one->id;
        auto item = d->model->appendItem(project);
        this->readFolder(item);
        item->setExpanded(true);
        return true;
    }
    return false;
}

void ResourceManagerPane::readFolder(ResourceManagerModelItem* item,int action){
    auto thread = BackendThread::getInstance();
    if(!thread->isRunning()){
        thread->start();
    }
    auto task = new ResourceManageReadFolderTask(d->model,item->path());
    if(action>-1){
        task->setType(action);
    }
    thread->appendTask(task);
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
    if(index.isValid()){
        auto item = static_cast<ResourceManagerModelItem*>(index.internalPointer());
        if(item->type()==ResourceManagerModelItem::File){
            //QString path = item->path();
            auto data = OpenEditorData{item->path(),0,0};
            Publisher::getInstance()->post(Type::M_OPEN_EDITOR,&data);
        }
    }
}

void ResourceManagerPane::onContextMenu(const QPoint& pos){
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
    QByteArray ba = mimeData->data(mime);
    d->actionPaste->setEnabled(false);
    if(!ba.isEmpty()){
        char c = ba.at(0);
        if(c==5 || c==2){
            d->actionPaste->setEnabled(true);
        }
    }
    if(one->pid()>0){
        d->actionUpload->setEnabled(true);
    }else{
        d->actionUpload->setEnabled(false);
    }
    if(type==ResourceManagerModelItem::Project){
        contextMenu.addAction(d->actionNew_File);
        contextMenu.addAction(d->actionNew_Folder);
        contextMenu.addAction(d->actionOpen_Folder);
        contextMenu.addAction(d->actionOpen_Terminal);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionPaste);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionUpload);
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
        contextMenu.addAction(d->actionUpload);
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
        contextMenu.addAction(d->actionUpload);
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

void ResourceManagerPane::onInsertReady(const QModelIndex& parent,bool isFile){
    auto one = static_cast<ResourceManagerModelItem*>(parent.internalPointer());
    ui->treeView->setExpanded(parent,true);
    auto model = static_cast<ResourceManagerModel*>(ui->treeView->model());
    QModelIndex index = model->insertItem(one,isFile?ResourceManagerModelItem::File:ResourceManagerModelItem::Folder);
    ui->treeView->editIndex(index);
}

void ResourceManagerPane::onActionTriggered(){
    QObject* sender = this->sender();
    QModelIndexList list = ui->treeView->selectionModel()->selectedRows();
    if(list.size()<=0){
        return ;
    }
    QModelIndex parent = list.at(0);
    auto one = static_cast<ResourceManagerModelItem*>(list.at(0).internalPointer());
    if(sender==d->actionNew_File){
        if(one->expanded()){
            this->onInsertReady(parent,true);
        }else{
            //add task
            this->readFolder(one,BackendThreadTask::ReadFolderAndInsertFile);
            one->setExpanded(true);
        }
    }else if(sender==d->actionNew_Folder){
        if(one->expanded()){
            this->onInsertReady(parent,false);
        }else{
            //add task
            this->readFolder(one,BackendThreadTask::ReadFolderAndInsertFolder);
            one->setExpanded(true);
        }
    }else if(sender==d->actionOpen_Folder){
        QDesktopServices::openUrl(QFileInfo(one->path()).absoluteDir().absolutePath());
    }else if(sender==d->actionOpen_File){
        //QString path = one->path();
        auto data = OpenEditorData{one->path(),0,0};
        Publisher::getInstance()->post(Type::M_OPEN_EDITOR,&data);
    }else if(sender==d->actionOpen_Terminal){

        QProcess process;
        QString directory = one->path();
        if(one->type()==ResourceManagerModelItem::File){
            QFileInfo fi(directory);
            directory = fi.absoluteDir().absolutePath();
        }

#ifdef Q_OS_WIN
        QStringList arguments;
        arguments << "/c" << "start" << "cmd.exe" << "/K" << QString("cd /d %1").arg(directory);
        QProcess::startDetached("cmd.exe", arguments);
#elif defined(Q_OS_MAC)
        QStringList arguments;
        arguments << "-e" << QString("tell application \"Terminal\" to do script \"cd %1 && exec $SHELL\"").arg(directory);
        QProcess::startDetached("osascript", arguments);
#elif defined(Q_OS_LINUX)
        QStringList arguments;
        arguments << QString("--working-directory=%1").arg(directory);
        QProcess::startDetached("gnome-terminal", arguments);
#else
        qWarning("Unsupported OS");
#endif



    }else if(sender==d->actionCut){
        QMimeData *mimeData = new QMimeData;
        QList<QUrl> urls;
        urls.append(QUrl::fromLocalFile(one->path()));
        mimeData->setUrls(urls);
        mimeData->setData(mime, QByteArray("\x02\x00\x00\x00", 4));
        QApplication::clipboard()->setMimeData(mimeData);
    }else if(sender==d->actionCopy){
        QMimeData *mimeData = new QMimeData;
        QList<QUrl> urls;
        urls.append(QUrl::fromLocalFile(one->path()));
        mimeData->setUrls(urls);
        mimeData->setData(mime, QByteArray("\x05\x00\x00\x00", 4));
        QApplication::clipboard()->setMimeData(mimeData);
    }else if(sender==d->actionPaste){
        const QMimeData *mimeData = QApplication::clipboard()->mimeData();
        if(mimeData->hasUrls()){
            QByteArray ba = mimeData->data(mime);
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
                                instance->post(new Event(Type::M_WILL_RENAME_FILE,(void*)&oPath));
                                if(!file.rename(dest)){
                                    pair.second = oPath;
                                }
                                instance->post(new Event(Type::M_RENAMED_FILE,(void*)&pair));
                            }else if(fi.isDir()){
                                instance->post(new Event(Type::M_WILL_RENAME_FOLDER,(void*)&oPath));
                                QDir dir(oPath);
                                QPair<QString,QString> pair = {oPath,dest};
                                if(!dir.rename(oPath,dest)){
                                    pair.second = oPath;
                                }
                                instance->post(new Event(Type::M_RENAMED_FOLDER,(void*)&pair));
                            }
                        }
                    }
                }
            }
        }

    }else if(sender==d->actionFind){
        OpenFindData data{0,{},one->path()};
        Publisher::getInstance()->post(Type::M_OPEN_FIND,&data);
    }else if(sender==d->actionRename){
        ui->treeView->editIndex(list.at(0));
    }else if(sender==d->actionDelete){
        if(MessageDialog::confirm(this,tr("Delete Confirm"),tr("Are you sure you want to delete the '%1'?\nYou can restore this file from the Recycle Bin.").arg(one->title()),QMessageBox::Ok|QMessageBox::Cancel)==QMessageBox::Ok){
            auto instance = Publisher::getInstance();
            const QString path = one->path();
            QPair<QString,QString> pair = {path,path};
            ResourceManagerModelItem::Type type = one->type();
            if(type==ResourceManagerModelItem::File){
                instance->post(new Event(Type::M_WILL_RENAME_FILE,(void*)&path));
                if(QFile::moveToTrash(one->path())){
                    instance->post(new Event(Type::M_DELETE_FILE,(void*)&pair));
                }else{
                    instance->post(new Event(Type::M_RENAMED_FILE,(void*)&pair));
                }
            }else if(type==ResourceManagerModelItem::Folder){
                //remove watch
                QStringList list = d->model->takeWatchDirectory(path,true);
                instance->post(new Event(Type::M_WILL_RENAME_FOLDER,(void*)&path));
                if(QFile::moveToTrash(one->path())){
                    instance->post(new Event(Type::M_DELETE_FOLDER,(void*)&path));
                }else{
                    foreach(auto one,list){
                        d->model->appendWatchDirectory(one);
                    }
                    instance->post(new Event(Type::M_RENAMED_FOLDER,(void*)&pair));
                }
            }
        }
    }else if(sender==d->actionClose_Project){
        if(MessageDialog::confirm(this,tr("Close Confirm"),tr("Are you sure you want to close '%1'?").arg(one->title()),QMessageBox::Ok|QMessageBox::Cancel)==QMessageBox::Ok){
            d->model->removeItem(one);
        }
    }else if(sender==d->actionCopy_Path){
        QApplication::clipboard()->setText(one->path());
    }else if(sender==d->actionUpload){
        UploadData data{one->pid(),one->type()==ResourceManagerModelItem::File,one->path(),{}};
        Publisher::getInstance()->post(Type::M_UPLOAD,&data);
    }
}

void ResourceManagerPane::onTopActionTriggered(){
    auto sender = static_cast<QAction*>(this->sender());
    if(sender==ui->actionCollapse){
        ui->treeView->collapseAll();
    }else if(sender==ui->actionExpand){
        ui->treeView->expandAll();
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
