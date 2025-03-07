#include "resource_manager_pane.h"
#include "ui_resource_manager_pane.h"
#include "resource_manager_model.h"
#include "resource_manager_model_item.h"
#include "resource_manager_tree_item_delegate.h"
#include "resource_manage_read_folder_task.h"
#include "locate_file_task.h"
#include "resource_manager_opened_model.h"
#include "core/event_bus/event.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event_data.h"
#include "core/backend_thread.h"
#include "storage/project_storage.h"
#include "storage/site_storage.h"
#include "storage/group_storage.h"
#include "storage/recent_storage.h"
#include "docking_pane_manager.h"
#include "docking_pane_layout_item_info.h"
#include "components/message_dialog.h"
#include "panes/code_editor/code_editor_manager.h"
#include "panes/code_editor/code_editor_pane.h"
#include "panes/file_transfer/file_transfer_pane.h"
#include "components/list_item_delegate.h"
#include "network/network_manager.h"
#include "common/utils.h"
#include "common.h"
#include "core/theme.h"
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QDesktopServices>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QMimeData>
#include <QProcess>
#include <QSplitter>
#include <QDebug>



const QString mime = "application/x-qt-windows-mime;value=\"Preferred DropEffect\"";

namespace ady{

ResourceManagerPane* ResourceManagerPane::instance = nullptr;

const QString ResourceManagerPane::PANE_ID = "ResourceManager";
const QString ResourceManagerPane::PANE_GROUP = "ResourceManager";




class ResourceManagerPanePrivate {
public:
    ResourceManagerModel* model;
    ResourceManagerOpenedModel* openedModel;
    QTreeView* openedView;
    QSplitter* splitter;


    //FolderReader* reader;
    QAction* actionNew_Folder;
    QAction* actionNew_File;
    QAction* actionOpen_Folder;
    QAction* actionAdd_Folder_To_Workspace;
    QAction* actionOpen_File;
    QAction* actionClose_Project;
    QAction* actionFind;
    QAction* actionOpen_Terminal;
    QAction* actionOpen_Embedded_Terminal;
    QAction* actionCopy;
    QAction* actionCut;
    QAction* actionPaste;
    QAction* actionRename;
    QAction* actionDelete;
    QAction* actionCopy_Path;
    //QAction* actionUpload;

    //QList<SiteRecord> sites;
    QMap<long long ,SiteRecord> sites;
};


ResourceManagerPane::ResourceManagerPane(QWidget *parent) :
    DockingPane(parent),
    ui(new Ui::ResourceManagerPane)
{
    Subscriber::reg();
    this->regMessageIds({Type::M_OPEN_PROJECT,Type::M_CLOSE_PROJECT,Type::M_FILE_CHANGED,Type::M_SITE_ADDED,Type::M_SITE_UPDATED,Type::M_SITE_DELETED,Type::M_RESOURCE_LOCATION});
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(tr("Resource Manager"));
    auto theme = Theme::getInstance();
    auto color = theme->color().name(QColor::HexRgb);
    auto secondaryColor = theme->secondaryColor().name(QColor::HexRgb);
    auto backgroundColor = theme->backgroundColor().name(QColor::HexRgb);
    auto textColor = theme->textColor().name(QColor::HexRgb);

    this->setStyleSheet("QToolBar{border:0px;}"
                        "QTreeView{border:0;background-color:"+backgroundColor+";color:"+textColor+";}"
                        ".ady--ResourceManagerPane>#widget{background-color:"+color+"}"
                        ".QSplitterHandle{background-color:"+secondaryColor+"}");

    ui->treeView->setItemDelegate(new ResourceManagerTreeItemDelegate(ui->treeView));
    ui->treeView->setEditTriggers(QAbstractItemView::EditKeyPressed);
    //init view
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->addMimeType(MM_UPLOAD);
    ui->treeView->setSupportDropFile(true);
    ui->treeView->setDragDropMode(QAbstractItemView::DragDrop);

    ui->comboBox->setItemDelegate(new ListItemDelegate(ui->comboBox));

    connect(ui->treeView,&QTreeView::customContextMenuRequested, this, &ResourceManagerPane::onContextMenu);
    connect(ui->treeView,&QTreeView::expanded,this,&ResourceManagerPane::onTreeItemExpanded);
    connect(ui->treeView,&QTreeView::collapsed,this,&ResourceManagerPane::onTreeItemCollapsed);
    connect(ui->treeView,&QTreeView::doubleClicked,this,&ResourceManagerPane::onTreeItemDClicked);
    connect(ui->treeView,&TreeView::dropFinished,this,&ResourceManagerPane::onDropAddFolder);

    connect(ui->actionExpand,&QAction::triggered,this,&ResourceManagerPane::onTopActionTriggered);
    connect(ui->actionCollapse,&QAction::triggered,this,&ResourceManagerPane::onTopActionTriggered);
    connect(ui->actionRefresh,&QAction::triggered,this,&ResourceManagerPane::onTopActionTriggered);
    connect(ui->actionHome,&QAction::triggered,this,&ResourceManagerPane::onTopActionTriggered);
    connect(ui->actionOpen_Files,&QAction::toggled,this,&ResourceManagerPane::onToggleOpend);


    d = new ResourceManagerPanePrivate;
    d->model = ResourceManagerModel::getInstance();
    ui->treeView->setModel(d->model);
    d->openedView = nullptr;
    d->splitter = nullptr;

    connect(d->model,&ResourceManagerModel::insertReady,this,&ResourceManagerPane::onInsertReady);
    connect(d->model,&ResourceManagerModel::itemsChanged,this,&ResourceManagerPane::onItemsChanged);
    connect(d->model,&ResourceManagerModel::locateSuccess,this,&ResourceManagerPane::onLocateSuccess);

    connect(ui->comboBox,&SearchComboBox::search,this,&ResourceManagerPane::onSearchFile);

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
    d->actionOpen_Folder = new QAction(QIcon(":/Resource/icons/FolderBrowserDialogControl_16x.svg"),tr("Open Folder"),this);
    d->actionAdd_Folder_To_Workspace = new QAction(QIcon(":/Resource/icons/OpenFolder_16x.svg"),tr("Add Folder to Workspace"),this);
    d->actionOpen_File = new QAction(QIcon(":/Resource/icons/OpenFile_16x.svg"),tr("Open File"),this);
    d->actionClose_Project = new QAction(tr("Close Project"),this);
    d->actionFind = new QAction(QIcon(":/Resource/icons/SearchFolderClosed_16x.svg"),tr("Find In Folder"),this);
    d->actionOpen_Terminal = new QAction(QIcon(":/Resource/icons/ImmediateWindow_16x.svg"),tr("Open Terminal"),this);
    d->actionOpen_Embedded_Terminal = new QAction(QIcon(":/Resource/icons/ImmediateWindow_16x.svg"),tr("Open Embedded Terminal"),this);
    d->actionCopy = new QAction(QIcon(":/Resource/icons/Copy_16x.svg"),tr("Copy"),this);
    d->actionCut = new QAction(QIcon(":/Resource/icons/Cut_16x.svg"),tr("Cut"),this);
    d->actionPaste = new QAction(QIcon(":/Resource/icons/Paste_16x.svg"),tr("Paste"),this);
    d->actionRename = new QAction(tr("Rename"),this);
    d->actionDelete = new QAction(QIcon(":/Resource/icons/Cancel_16x.svg"),tr("Delete"),this);
    d->actionCopy_Path = new QAction(tr("Copy Path"),this);
    //d->actionUpload = new QAction(QIcon(":/Resource/icons/BatchCheckIn_16x.svg"),tr("Upload"),this);

    d->actionCut->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_X));
    d->actionCopy->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_C));
    d->actionPaste->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_V));
    d->actionRename->setShortcut(QKeySequence(Qt::Key_F2));
    d->actionFind->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_F));

    connect(d->actionNew_Folder,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionNew_File,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionOpen_Folder,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionAdd_Folder_To_Workspace,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionOpen_File,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionClose_Project,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionFind,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionOpen_Terminal,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionOpen_Embedded_Terminal,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionCopy,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionPaste,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionRename,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionDelete,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionCopy_Path,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    connect(d->actionCut,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);
    //connect(d->actionUpload,&QAction::triggered,this,&ResourceManagerPane::onActionTriggered);


}

QString ResourceManagerPane::id(){
    return PANE_ID;
}

QString ResourceManagerPane::group(){
    return PANE_GROUP;
}

bool ResourceManagerPane::onReceive(Event* e){
    const QString id = e->id();
    if(id==Type::M_OPEN_PROJECT){
        auto proj = e->toJsonOf<ProjectRecord>().toObject();
        long long id = proj.find("id")->toInt(0);
        auto project = new ProjectRecord();
        if(id>0){
            ProjectStorage projectStorage;
            auto one = projectStorage.one(id);
            if(one.id>0){
                project->id = one.id;
                project->path = one.path;
                project->name = one.name;
            }
        }
        if(project->id==0){
            const QString path = proj.find("path")->toString();
            if(!path.isEmpty()){
                project->path = path;
                QFileInfo fi(path);
                project->name = fi.fileName();
            }
        }
        if(project->id==0 && project->path.isEmpty()){
            delete project;
            return false;
        }
        //find proj
        auto exists = d->model->findProject(project->path);
        if(exists!=nullptr){
            //has opened
            return true;
        }

        //add to recent project
        RecentStorage().add(project->name,project->path,project->id);


        auto item = d->model->appendItem(project);
        if(project->id>0){
            //find all sites
            SiteStorage siteStorage;
            auto list = siteStorage.list(project->id,1);//status==1 all sites
            for(auto one:list){
                if(!d->sites.contains(one.id)){
                    d->sites.insert(one.id,one);
                }
            }
        }

        auto arr = proj.find("opened")->toArray();
        if(arr.size()>0){
            QStringList opendlist;
            for(auto one:arr){
                if(one.toString().isEmpty()==false){
                    opendlist << one.toString();
                }
            }
            if(opendlist.length()>0){
                item->setOpenList(opendlist);
            }
        }

        this->readFolder(item);
        item->setExpanded(true);
        return true;


    }else if(id==Type::M_CLOSE_PROJECT){
        auto proj = e->toJsonOf<CloseProjectData>().toObject();
        if(proj.isEmpty()){
            //from menu
            //close selected or first
            ResourceManagerModelItem* item = nullptr;
            auto list = ui->treeView->selectionModel()->selectedRows();
            if(list.size()==0){
                auto root = d->model->rootItem();
                if(root->childrenCount()>0){
                    item = root->childAt(0);
                }
            }else{
                item = static_cast<ResourceManagerModelItem*>(list.at(0).internalPointer());
            }
            if(item!=nullptr && item->type()==ResourceManagerModelItem::Project){
                this->closeProject(item);
            }
        }
    }else if(id==Type::M_SITE_ADDED){
        auto site = e->toJsonOf<SiteRecord>().toObject();
        auto id = site.find("id")->toInt(0);
        if(id!=0){
            auto one = SiteStorage().one(id);
            if(one.id!=0 && one.status==1){
                if(!d->sites.contains(one.id)){
                    d->sites.insert(one.id,one);
                }
            }
        }
    }else if(id==Type::M_SITE_UPDATED){
        auto site = e->toJsonOf<SiteRecord>().toObject();
        auto id = site.find("id")->toInt(0);
        if(id!=0){
            auto one = SiteStorage().one(id);
            if(one.id>0){
                if(one.status!=1){
                    //remove
                    d->sites.remove(one.id);
                }else{
                    if(!d->sites.contains(one.id)){
                        //update
                        d->sites.insert(one.id,one);
                    }
                }
            }
        }
    }else if(id==Type::M_SITE_DELETED){
        auto site = e->toJsonOf<SiteRecord>().toObject();
        auto id = site.find("id")->toInt(0);
        if(id!=0){
            d->sites.remove(id);
        }
    }else if(id==Type::M_RESOURCE_LOCATION){
        this->locationFile();
    }
    return false;
}

void ResourceManagerPane::readFolder(ResourceManagerModelItem* item,int action){
    auto thread = BackendThread::getInstance();
    if(!thread->isRunning()){
        thread->start();
    }
    auto path = item->path();
    auto list = item->openList();
    if(list.size()>0){

        path += ResourceManageReadFolderTask::divider + list.join(ResourceManageReadFolderTask::divider);
        for(auto one:list){
            d->model->appendWatchDirectory(one);
        }
    }
    //qDebug()<<"path:"<<path;
    auto task = new ResourceManageReadFolderTask(d->model,path);
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
            item->setState(ResourceManagerModelItem::Expand);
        }
    }
}

void ResourceManagerPane::onTreeItemCollapsed(const QModelIndex& index){
    if(index.isValid()){
        auto item = static_cast<ResourceManagerModelItem*>(index.internalPointer());
        if(item!=nullptr){
            item->setState(ResourceManagerModelItem::Collapse);
        }
    }
}


void ResourceManagerPane::onTreeItemDClicked(const QModelIndex& index){
    if(index.isValid()){
        auto item = static_cast<ResourceManagerModelItem*>(index.internalPointer());
        if(item->type()==ResourceManagerModelItem::File){
            //QString path = item->path();
            auto data = OpenEditorData{item->path(),0,0,false};
            Publisher::getInstance()->post(Type::M_OPEN_EDITOR,&data);

            this->showStatusMessage(item->pid(),item->path());
        }
    }
}

void ResourceManagerPane::onContextMenu(const QPoint& pos){
    QMenu contextMenu(this);
    QModelIndexList list = ui->treeView->selectionModel()->selectedRows();
    QModelIndex index = ui->treeView->indexAt(pos);
    if(!index.isValid()){
        if(list.size()==0){
            return ;
        }
        index = list.at(0);
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
    /*if(one->pid()>0){
        d->actionUpload->setEnabled(true);
    }else{
        d->actionUpload->setEnabled(false);
    }*/
    if(type==ResourceManagerModelItem::Project){
        contextMenu.addAction(d->actionNew_File);
        contextMenu.addAction(d->actionNew_Folder);
        contextMenu.addAction(d->actionOpen_Folder);
        contextMenu.addAction(d->actionOpen_Terminal);
        contextMenu.addAction(d->actionOpen_Embedded_Terminal);

        contextMenu.addSeparator();
        contextMenu.addAction(d->actionPaste);
        contextMenu.addSeparator();
        //contextMenu.addAction(d->actionUpload);
        if(one->pid()>0){
            auto uploadM = this->attchUploadMenu(&contextMenu,one->pid());
            if(uploadM!=nullptr){
                contextMenu.addMenu(uploadM);
            }
        }
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionFind);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionAdd_Folder_To_Workspace);
        contextMenu.addAction(d->actionClose_Project);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionCopy_Path);
    }else if(type==ResourceManagerModelItem::Folder){
        contextMenu.addAction(d->actionNew_File);
        contextMenu.addAction(d->actionNew_Folder);
        contextMenu.addAction(d->actionOpen_Folder);
        contextMenu.addAction(d->actionOpen_Terminal);
        contextMenu.addAction(d->actionOpen_Embedded_Terminal);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionCut);
        contextMenu.addAction(d->actionCopy);
        contextMenu.addAction(d->actionPaste);
        contextMenu.addSeparator();
        //contextMenu.addAction(d->actionUpload);
        if(one->pid()>0){
            auto uploadM = this->attchUploadMenu(&contextMenu,one->pid());
            if(uploadM!=nullptr){
                contextMenu.addMenu(uploadM);
            }
        }
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionFind);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionRename);
        contextMenu.addAction(d->actionDelete);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionAdd_Folder_To_Workspace);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionCopy_Path);
    }else if(type==ResourceManagerModelItem::File){
        contextMenu.addAction(d->actionOpen_File);
        contextMenu.addAction(d->actionOpen_Terminal);
        contextMenu.addAction(d->actionOpen_Embedded_Terminal);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionCut);
        contextMenu.addAction(d->actionCopy);
        contextMenu.addSeparator();
        //contextMenu.addAction(d->actionUpload);
        if(one->pid()>0){
            auto uploadM = this->attchUploadMenu(&contextMenu,one->pid());
            if(uploadM!=nullptr){
                contextMenu.addMenu(uploadM);
            }
        }
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionRename);
        contextMenu.addAction(d->actionDelete);
        contextMenu.addSeparator();
        contextMenu.addAction(d->actionAdd_Folder_To_Workspace);
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
    ui->treeView->selectionModel()->select(index,QItemSelectionModel::ClearAndSelect);
    ui->treeView->scrollTo(index, QAbstractItemView::EnsureVisible);
}

void ResourceManagerPane::onItemsChanged(const QString& path){
    auto item = d->model->rootItem();
    int count = item->childrenCount();
    for(int i=0;i<count;i++){
        auto proj = item->childAt(i);
        auto node = proj->findChild(path);
        auto list = proj->openList();
        if(list.size()>0){
            if(proj->path() == path){
                QModelIndex index = d->model->toIndex(proj);
                ui->treeView->expand(index);
                proj->removeOpenList(path);
            }
            if(node!=nullptr){
                QModelIndex index = d->model->toIndex(node);
                ui->treeView->expand(index);
                proj->removeOpenList(path);
            }
        }
        if(node!=nullptr){
            auto index = d->model->currentItem(node);
            if(index.isValid()){

                ui->treeView->selectionModel()->select(index,QItemSelectionModel::ClearAndSelect);
                ui->treeView->scrollTo(index, QAbstractItemView::EnsureVisible);

            }
        }
    }
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
        if(one->type()==ResourceManagerModelItem::File){
            QDesktopServices::openUrl(QFileInfo(one->path()).path());
        }else{
            QDesktopServices::openUrl(one->path());
        }
    }else if(sender==d->actionAdd_Folder_To_Workspace){
        QString path = QFileDialog::getExistingDirectory(this, tr("Select Folder"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if(!path.isEmpty()){
            ProjectRecord record;
            record.path = path;
            QFileInfo fi(path);
            record.name = fi.fileName();
            Publisher::getInstance()->post(Type::M_OPEN_PROJECT,(void*)&record);
        }

    }else if(sender==d->actionOpen_File){
        //QString path = one->path();
        auto data = OpenEditorData{one->path(),0,0,false};
        Publisher::getInstance()->post(Type::M_OPEN_EDITOR,&data);

        this->showStatusMessage(one->pid(),one->path());

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
    }else if(sender==d->actionOpen_Embedded_Terminal){
        QString directory = one->path();
        if(one->type()==ResourceManagerModelItem::File){
            QFileInfo fi(directory);
            directory = fi.absoluteDir().absolutePath();
        }
        Publisher::getInstance()->post(Type::M_OPEN_TERMINAL,&directory);
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
            QFileInfo fi(path);
            const QString parentPath = fi.path();
            QStringList parentList = d->model->takeWatchDirectory(parentPath,false);//remove parent dir watcher
            if(type==ResourceManagerModelItem::File){
                instance->post(new Event(Type::M_WILL_RENAME_FILE,(void*)&path));
                if(QFile::moveToTrash(one->path())){
                    instance->post(new Event(Type::M_DELETE_FILE,(void*)&pair));
                    d->model->removeItem(one);
                }else{
                    instance->post(new Event(Type::M_RENAMED_FILE,(void*)&pair));
                }

            }else if(type==ResourceManagerModelItem::Folder){
                //remove watch
                QStringList list = d->model->takeWatchDirectory(path,true);
                instance->post(new Event(Type::M_WILL_RENAME_FOLDER,(void*)&path));
                if(QFile::moveToTrash(one->path())){
                    instance->post(new Event(Type::M_DELETE_FOLDER,(void*)&path));
                    d->model->removeItem(one);
                }else{
                    //folder delete faild ;restore dir watcher
                    foreach(auto one,list){
                        d->model->appendWatchDirectory(one);
                    }
                    instance->post(new Event(Type::M_RENAMED_FOLDER,(void*)&pair));
                }
            }
            //restore parent dir watcher
            foreach(auto one,parentList){
                d->model->appendWatchDirectory(one);
            }
        }
    }else if(sender==d->actionClose_Project){
        /*if(MessageDialog::confirm(this,tr("Close Confirm"),tr("Are you sure you want to close '%1'?").arg(one->title()),QMessageBox::Ok|QMessageBox::Cancel)==QMessageBox::Ok){
            d->model->removeItem(one);
        }*/
        this->closeProject(one);
    }else if(sender==d->actionCopy_Path){
        QApplication::clipboard()->setText(one->path());
    }
}

void ResourceManagerPane::onTopActionTriggered(){
    auto sender = static_cast<QAction*>(this->sender());
    if(sender==ui->actionCollapse){
        ui->treeView->collapseAll();
    }else if(sender==ui->actionExpand){
        ui->treeView->expandAll();
    }else if(sender==ui->actionRefresh){
        //file loaction
        this->locationFile();

    }
}

void ResourceManagerPane::onUploadToSite(){
    auto sender = static_cast<QAction*>(this->sender());
    auto v = sender->data();
    bool ok = false;
    long long siteid = v.toLongLong(&ok);
    if(ok){
        this->uploadSelectedFile(siteid);
    }
}

void ResourceManagerPane::onUploadToGroup(){
    auto sender = static_cast<QAction*>(this->sender());
    auto v = sender->data().toString();
    QStringList list = v.split(":");
    if(list.size()==2){
        bool ok = false;
        long long pid = list.at(0).toLongLong(&ok);
        if(ok){
            long long groupid = list.at(1).toLongLong(&ok);
            if(ok){
                for(auto one:d->sites){
                    if(one.pid==pid && one.groupid == groupid){
                        this->uploadSelectedFile(one.id);
                    }
                }
            }
        }
    }
}

void ResourceManagerPane::onUploadToAll(){
    auto sender = static_cast<QAction*>(this->sender());
    auto v = sender->data();
    bool ok = false;
    long long pid = v.toLongLong(&ok);
    if(ok){
        for(auto one:d->sites){
            if(one.pid==pid){
                this->uploadSelectedFile(one.id);
            }
        }
    }
}

void ResourceManagerPane::uploadSelectedFile(long long siteid){
    auto list = ui->treeView->selectionModel()->selectedRows();
    if(list.size()>0){
        //open file transfer
        auto instance = Publisher::getInstance();
        QString paneGroup = FileTransferPane::PANE_GROUP;
        instance->post(Type::M_OPEN_PANE,&paneGroup);
        auto index = list.at(0);
        auto item = static_cast<ResourceManagerModelItem*>(index.internalPointer());
        //not set dest ,should match remote
        UploadData data{item->pid(),siteid,item->type()==ResourceManagerModelItem::File,item->path(),{}};
        instance->post(Type::M_UPLOAD,&data);
    }
}



void ResourceManagerPane::onSearchFile(const QString& text){
    auto delegate = static_cast<ResourceManagerTreeItemDelegate*>(ui->treeView->itemDelegate());
    delegate->setSearchText(text);
    ui->treeView->update();
}

void ResourceManagerPane::onDropAddFolder(const QMimeData* data){
    if(data->hasUrls()){
        QList<QUrl> urls = data->urls();
        for(auto url:urls){
            QFileInfo fi(url.toLocalFile());
            if(fi.isDir()){
                ProjectRecord record;
                record.path = fi.absoluteFilePath();
                record.name = fi.fileName();
                Publisher::getInstance()->post(Type::M_OPEN_PROJECT,(void*)&record);
            }
        }
    }
}

void ResourceManagerPane::onToggleOpend(bool checked){
    auto widget = this->centerWidget();
    if(checked){
        //open
        if(d->splitter==nullptr){
            d->splitter = new QSplitter(Qt::Vertical,widget);
        }
        ui->treeView->setParent(d->splitter);
        d->splitter->insertWidget(0,ui->treeView);
        if(d->openedView==nullptr){
            d->openedView = new QTreeView(d->splitter);
            d->openedView->setItemDelegate(new ResourceManagerOpenedItemDelegate(d->openedView));
            d->openedView->setHeaderHidden(true);
            connect(d->openedView,&QTreeView::clicked,this,&ResourceManagerPane::onOpenedSelected);
            d->openedModel = ResourceManagerOpenedModel::getInstance();
            d->openedView->setModel(d->openedModel);
            d->openedModel->setDataSource(CodeEditorManager::openedFiles());
            d->splitter->addWidget(d->openedView);
        }
        ui->verticalLayout->addWidget(d->splitter,1);
        d->splitter->setSizes({1,1});
        d->splitter->show();
    }else{
        //close
        ui->treeView->setParent(widget);
        ui->verticalLayout->addWidget(ui->treeView,1);
        if(d->splitter!=nullptr){
            ui->verticalLayout->removeWidget(d->splitter);
            d->splitter->hide();
        }
    }
}

void ResourceManagerPane::onOpenedSelected(const QModelIndex& index){
    int row = index.row();
    auto item = d->openedModel->itemAt(row);
    //qDebug()<<"onOpenedSelected"<<item.second;
    OpenEditorData data{item.second,0,0,true};
    Publisher::getInstance()->post(Type::M_OPEN_EDITOR,&data);
}

void ResourceManagerPane::onLocateSuccess(const QString& path,bool recursion){
    auto index = d->model->locate(path);
    if(index.isValid()){
        auto item = static_cast<ResourceManagerModelItem*>(index.internalPointer());
        auto type = item->type();
        if(type==ResourceManagerModelItem::File){
            //ok
            //expanded
            //auto parent = d->model->toIndex(item->parent());
            auto parent = index.parent();
            while(parent.isValid() && ui->treeView->isExpanded(parent)==false){
                ui->treeView->setExpanded(parent,true);
                parent = parent.parent();
            }
            ui->treeView->selectionModel()->select(index,QItemSelectionModel::ClearAndSelect);
            ui->treeView->scrollTo(index, QAbstractItemView::EnsureVisible);
        }else{
            //read folder to find path
            if(recursion)
                BackendThread::getInstance()->appendTask(new LocateFileTask(d->model,item->path(),path));
        }

        //send message to statusbar for display project name
        this->showStatusMessage(item->pid(),path);
    }
}

QMenu* ResourceManagerPane::attchUploadMenu(QMenu* parent,long long id){
    QMenu* uploadM = nullptr;
    QList<long long>gids;
    int total = 0;
    for(auto one:d->sites){
        if(one.pid==id){
            if(uploadM==nullptr){
                uploadM = new QMenu(parent);
                uploadM->setTitle(tr("Upload"));
            }
            auto action = uploadM->addAction(QIcon(":/Resource/icons/RemoteServer_16x.svg"),one.name,this,SLOT(onUploadToSite()));
            action->setData(one.id);//site id
            total += 1;
            if(!gids.contains(one.groupid)){
                gids.append(one.groupid);
            }
        }
    }

    if(total>1){
        //upload group
        QList<GroupRecord> grouplist = GroupStorage().list(id);
        if(grouplist.size()>0){
            uploadM->addSeparator();
        }
        for(auto one:grouplist){
            if(gids.contains(one.id)){
                auto action = uploadM->addAction(QIcon(":/Resource/icons/RemoteServer_16x.svg"),one.name,this,SLOT(onUploadToGroup()));
                QString data = QString::fromUtf8("%1:%2").arg(id).arg(one.id);//projectid and groupid
                action->setData(data);//
            }
        }
        //upload all
        uploadM->addSeparator();
        auto action = uploadM->addAction(QIcon(":/Resource/icons/RemoteServer_16x.svg"),tr("All Sites"),this,SLOT(onUploadToAll()));
         action->setData(id);//project id
    }


    return uploadM;
}

void ResourceManagerPane::closeProject(ResourceManagerModelItem* item){
    if(MessageDialog::confirm(this,tr("Close Confirm"),tr("Are you sure you want to close '%1'?").arg(item->title()),QMessageBox::Ok|QMessageBox::Cancel)==QMessageBox::Ok){
        long long id = item->pid();
        const QString path = item->path();
        d->model->removeItem(item);
        CloseProjectData data{id,path};
        Publisher::getInstance()->post(Type::M_CLOSE_PROJECT_NOTIFY,&data);

        //remove network request
        auto instance = NetworkManager::getInstance();
        for(auto one:d->sites){
            if(one.pid==id){
                instance->remove(one.id);
            }
        }
    }
}

void ResourceManagerPane::locationFile(){
    auto pane = CodeEditorManager::getInstance()->current();
    if(pane!=nullptr){
        auto path = pane->path();
        d->model->setCurrentPath(path);
        this->onLocateSuccess(path,true);
    }
}

void ResourceManagerPane::showStatusMessage(long long id,const QString& path){
    ResourceManagerModelItem* proj = nullptr;
    if(id){
        //find project name
        proj = d->model->findProject(id);
    }else{
        //find project name by path previous
        proj = d->model->findProjectPath(path);
    }
    if(proj!=nullptr){
        QString title = proj->title();
        Publisher::getInstance()->post(Type::M_MESSAGE,&title);
    }else{
        Publisher::getInstance()->post(Type::M_READY);
    }
}

ResourceManagerPane* ResourceManagerPane::getInstance(){
    return instance;
}

ResourceManagerPane* ResourceManagerPane::open(DockingPaneManager* dockingManager,bool active){
    if(instance==nullptr){
        instance = new ResourceManagerPane(dockingManager->widget());
        DockingPaneLayoutItemInfo* item = dockingManager->createPane(instance,DockingPaneManager::S_Left,active);
        item->setManualSize(300);
    }
    return instance;
}

ResourceManagerPane* ResourceManagerPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    if(instance==nullptr){
        instance = new ResourceManagerPane(dockingManager->widget());
        return instance;
    }else{
        return nullptr;
    }
}

}
