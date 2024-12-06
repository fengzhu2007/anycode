#include "server_manage_pane.h"
#include "ui_server_manage_pane.h"
#include <docking_pane_manager.h>
#include <docking_pane_container.h>
#include <docking_workbench.h>
#include <docking_pane_layout_item_info.h>
#include "core/event_bus/event.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event_data.h"
#include "storage/project_storage.h"
#include "server_manage_model.h"
#include "network/network_manager.h"
#include "server_request_thread.h"
#include "new_folder_dialog.h"
#include "permission_dialog.h"
#include "panes/file_transfer/file_transfer_pane.h"
#include <w_toast.h>
#include "components/message_dialog.h"
#include "server_client_pane.h"
#include "site_quick_manager_dialog.h"
#include "common.h"
#include <QMenu>
#include <QThread>
#include <QFileDialog>
#include <QDebug>
namespace ady{

ServerManagePane* ServerManagePane::instance = nullptr;

const QString ServerManagePane::PANE_ID = "ServerManager";
const QString ServerManagePane::PANE_GROUP = "ServerManager";
const QString ServerManagePane::PANE_TITLE = tr("Server Manager");



class ServerManagePanePrivate{
public:
    QList<ServerRequestThread*> list;
};

ServerManagePane::ServerManagePane(QWidget *parent) :
    DockingPane(parent),
    ui(new Ui::ServerManagePane)
{
    Subscriber::reg();
    this->regMessageIds({Type::M_UPLOAD,Type::M_OPEN_PROJECT,Type::M_CLOSE_PROJECT_NOTIFY,Type::M_NOTIFY_REFRESH_TREE,Type::M_SITE_ADDED,Type::M_SITE_UPDATED,Type::M_SITE_DELETED});
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(PANE_TITLE);
    this->setStyleSheet("QToolBar{border:0px;}"
                        "QTreeView{border:0;background-color:#f5f5f5}"
                        ".ady--ServerManagePane>#widget{background-color:#EEEEF2}");

    d = new ServerManagePanePrivate;

    auto model = new ServerManageModel(ui->treeView);

    connect(model,&ServerManageModel::rename,this,&ServerManagePane::onRename);

    ui->treeView->setModel(model);
    ui->treeView->setColumnWidth(ServerManageModel::Name,240);
    ui->treeView->addMimeType(MM_UPLOAD);
    ui->treeView->setSupportDropFile(true);
    ui->treeView->setDragDropMode(QAbstractItemView::DragDrop);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView,&QTreeView::customContextMenuRequested, this, &ServerManagePane::onContextMenu);
    connect(ui->treeView,&QTreeView::expanded,this,&ServerManagePane::onTreeItemExpanded);
    connect(ui->treeView,&TreeView::dropFinished,this,&ServerManagePane::onDropUpload);
    //connect(ui->treeView,&QTreeView::doubleClicked,this,&ServerManagePane::onTreeItemDClicked);


    connect(ui->actionConnect_Server,&QAction::triggered,this,&ServerManagePane::onActionTriggered);
    connect(ui->actionRefresh,&QAction::triggered,this,&ServerManagePane::onActionTriggered);
    connect(ui->actionDock_To_Client,&QAction::triggered,this,&ServerManagePane::onActionTriggered);
    //connect(ui->actionOpen,&QAction::triggered,this,&ServerManagePane::onActionTriggered);
    connect(ui->actionDownload,&QAction::triggered,this,&ServerManagePane::onActionTriggered);
    connect(ui->actionDownload_To,&QAction::triggered,this,&ServerManagePane::onActionTriggered);
    connect(ui->actionPermissions,&QAction::triggered,this,&ServerManagePane::onActionTriggered);
    connect(ui->actionRename,&QAction::triggered,this,&ServerManagePane::onActionTriggered);
    connect(ui->actionDelete,&QAction::triggered,this,&ServerManagePane::onActionTriggered);
    connect(ui->actionNew_Folder,&QAction::triggered,this,&ServerManagePane::onActionTriggered);

    this->initView();
}

ServerManagePane::~ServerManagePane()
{
    Subscriber::unReg();
    for(auto one:d->list){
        //one->interrupt();
        one->requestInterruption();
        one->quit();
        //one->wait();
    }
    delete ui;
    delete d;
    instance = nullptr;
}

void ServerManagePane::initView(){

}

QString ServerManagePane::id(){
    return PANE_ID;
}

QString ServerManagePane::group(){
    return PANE_GROUP;
}

bool ServerManagePane::onReceive(Event* e) {
    const QString id = e->id();
    if(id==Type::M_UPLOAD){

    }else if(id==Type::M_DOWNLOAD){

    }else if(id==Type::M_OPEN_PROJECT){
        auto proj = e->toJsonOf<ProjectRecord>().toObject();
        long long id = proj.find("id")->toInt(0);
        if(id>0){
            ProjectStorage projectStorage;
            auto one = projectStorage.one(id);
            if(one.id>0){
                auto model = static_cast<ServerManageModel*>(ui->treeView->model());
                model->openProject(one.id,one.name);
            }
        }
        return true;
    }else if(id==Type::M_CLOSE_PROJECT_NOTIFY){
        auto proj = e->toJsonOf<CloseProjectData>().toObject();
        long long id = proj.find("id")->toInt(0);
        if(id>0){
            auto model = static_cast<ServerManageModel*>(ui->treeView->model());
            model->removeProject(id);
        }
    }else if(id==Type::M_NOTIFY_REFRESH_TREE){
        auto data = e->toJsonOf<ServerRefreshData>().toObject();
        long long id = data.find("id")->toInt(0);
        const QString path = data.find("path")->toString();
        if(id==0 || path.isEmpty()){
            return false;
        }
        auto model = static_cast<ServerManageModel*>(ui->treeView->model());
        auto site = model->find(id,false);
        if(site!=nullptr){
            ServerManageModelItem* child = nullptr;
            if(path==site->path()){
                child = site;
            }else{
                child = site->findChild(path);
            }
            if(child!=nullptr){
                int cmd = data.find("cmd")->toInt(0);
                if((cmd==ServerRequestThread::Link || cmd==ServerRequestThread::List || cmd==ServerRequestThread::Refresh) && child->expanded()){
                    return true;
                }
                auto array = data.find("list")->toArray();
                QList<FileItem> list;
                for(auto one:array){
                    list << FileItem::fromJson(one.toObject());
                }
                child->setExpanded(true);
                model->refreshItems(list,child);
                return true;
            }
        }
    }else if(id==Type::M_SITE_ADDED){
        auto site = e->toJsonOf<SiteRecord>().toObject();
        auto id = site.find("id")->toInt(0);
        if(id!=0){
            //Quick Connect
            auto one = SiteStorage().one(id);
            if(one.id!=0 && one.status==1){
                auto model = static_cast<ServerManageModel*>(ui->treeView->model());
                model->addSite(one);
                if(one.pid==0){
                    auto index = model->index(0,0);
                    ui->treeView->expand(index);
                }
            }
        }
    }else if(id==Type::M_SITE_UPDATED){
        auto site = e->toJsonOf<SiteRecord>().toObject();
        auto id = site.find("id")->toInt(0);
        if(id!=0){
            auto one = SiteStorage().one(id);
            if(one.id>0){
                auto model = static_cast<ServerManageModel*>(ui->treeView->model());
                if(one.status!=1){
                    //remove
                    model->removeSite(one.id);
                }else{
                    auto r = model->find(one.id,false);
                    if(r){
                        model->updateSite(one);
                    }else{
                        //add
                        model->addSite(one);
                        if(one.pid==0){
                            auto index = model->index(0,0);
                            ui->treeView->expand(index);
                        }
                    }
                }
            }
        }
    }else if(id==Type::M_SITE_DELETED){
        auto site = e->toJsonOf<SiteRecord>().toObject();
        auto id = site.find("id")->toInt(0);
        if(id!=0){
            auto model = static_cast<ServerManageModel*>(ui->treeView->model());
            model->removeSite(id);
        }
    }
    return false;
}

void ServerManagePane::addThread(ServerRequestThread* thread){
    connect(thread,&ServerRequestThread::finished,this,&ServerManagePane::onThreadFinished);
    d->list.push_back(thread);
}


void ServerManagePane::onContextMenu(const QPoint& pos){
    QMenu contextMenu(this);
    auto model = ui->treeView->selectionModel();
    QModelIndex index = model->currentIndex();
    int total = model->model()->rowCount();
    if(index.isValid()){
        auto item = static_cast<ServerManageModelItem*>(index.internalPointer());
        ServerManageModelItem::Type type = item->type();
        if(type==ServerManageModelItem::Server || type==ServerManageModelItem::Folder){
            contextMenu.addAction(ui->actionRefresh);
            contextMenu.addAction(ui->actionDock_To_Client);
            contextMenu.addSeparator();
            contextMenu.addAction(ui->actionNew_Folder);
        }
        if(type==ServerManageModelItem::File){
            //contextMenu.addAction(ui->actionOpen);
        }
        contextMenu.addSeparator();
        contextMenu.addAction(ui->actionDownload);
        contextMenu.addAction(ui->actionDownload_To);
        contextMenu.addSeparator();
        if(type==ServerManageModelItem::File || type==ServerManageModelItem::Folder){
            contextMenu.addAction(ui->actionPermissions);
        }
        contextMenu.addAction(ui->actionRename);
        contextMenu.addAction(ui->actionDelete);
        contextMenu.exec(QCursor::pos());
    }

}

void ServerManagePane::connectServer(long long id,const QString& path){
    auto req = NetworkManager::getInstance()->request(id);
    if(req!=nullptr){
        auto thread = new ServerRequestThread(req,ServerRequestThread::Link,new QString(path));
        connect(thread,&ServerRequestThread::finished,thread,&ServerRequestThread::deleteLater);
        connect(thread,&ServerRequestThread::resultReady, this, &ServerManagePane::onNetworkResponse);
        this->addThread(thread);
        thread->start();
    }
}

void ServerManagePane::cdDir(long long id,const QString& path,bool refresh){
    auto req = NetworkManager::getInstance()->request(id);
    if(req!=nullptr){
        auto thread = new ServerRequestThread(req,refresh?ServerRequestThread::Refresh:ServerRequestThread::List,new QString(path));
        connect(thread,&ServerRequestThread::finished,thread,&ServerRequestThread::deleteLater);
        connect(thread,&ServerRequestThread::resultReady, this, &ServerManagePane::onNetworkResponse);
        this->addThread(thread);
        thread->start();
    }
}

void ServerManagePane::deleteFiles(long long id,const QStringList& list){
    auto req = NetworkManager::getInstance()->request(id);
    if(req!=nullptr){
        auto thread = new ServerRequestThread(req,ServerRequestThread::Delete,new QStringList(list));
        connect(thread,&ServerRequestThread::finished,thread,&ServerRequestThread::deleteLater);
        connect(thread,&ServerRequestThread::resultReady, this, &ServerManagePane::onNetworkResponse);
        connect(thread,&ServerRequestThread::message, this, &ServerManagePane::onMessage);
        connect(thread,&ServerRequestThread::output, this, &ServerManagePane::onOutput);
        this->addThread(thread);
        thread->start();
    }
}

void ServerManagePane::onNetworkResponse(NetworkResponse* response,int command,int result){
    if(result==ServerRequestThread::Unsppport){
        wToast::showText(tr("This operation is not supported."));
        return ;
    }
    auto model = static_cast<ServerManageModel*>(ui->treeView->model());
    if(command==ServerRequestThread::Link){
        long long id = response->id;
        auto item = model->find(id,false);
        if(response->status()){
            //list default dir
            auto list = response->parseList();
            auto dir = response->params["dir"].toString();
            if(item!=nullptr){
                item->setExpanded(true);
                item->setLoading(false);
                model->appendItems(list,item);
            }
            ServerRefreshData data{command,id,dir,list};
            Publisher::getInstance()->post(Type::M_NOTIFY_REFRESH_LIST,&data);
        }else{
            if(item!=nullptr){
                item->setLoading(false);
                model->changeItem(item);
            }
            wToast::showText(response->errorMsg);
        }
        this->output(response);
    }else if(command==ServerRequestThread::List){
        auto dir = response->params["dir"].toString();
        auto item = model->find(response->id,dir);
        if(response->status()){
            auto list = response->parseList();
            if(item!=nullptr){
                item->setExpanded(true);
                item->setLoading(false);
                model->appendItems(list,item);
            }
            ServerRefreshData data{command,response->id,dir,list};
            Publisher::getInstance()->post(Type::M_NOTIFY_REFRESH_LIST,&data);
        }else{
            if(item!=nullptr){
                item->setLoading(false);
                model->changeItem(item);
            }
            wToast::showText(response->errorMsg);
        }
        this->output(response);
    }else if(command==ServerRequestThread::Refresh || command==ServerRequestThread::NewFolder || command==ServerRequestThread::Delete){
        auto item = model->find(response->id);

        if(response->status()){
            auto list = response->parseList();
            auto dir = response->params["dir"].toString();

            if(item!=nullptr){
                if(item->path()!=dir){
                    item = item->findChild(dir);
                }
                if(item!=nullptr){
                    //clear all children
                    item->setExpanded(true);
                    item->setLoading(false);
                    model->changeItem(item);
                    model->refreshItems(list,item);
                }
            }
            ServerRefreshData data{command,response->id,dir,list};
            Publisher::getInstance()->post(Type::M_NOTIFY_REFRESH_LIST,&data);
        }else{
            if(item!=nullptr){
                item->setLoading(false);
                model->changeItem(item);
            }
            wToast::showText(response->errorMsg);
        }
        this->output(response);
    }else if(command==ServerRequestThread::Rename || command==ServerRequestThread::Chmod){
        auto dir = response->params["dir"].toString();
        auto item = model->find(response->id);
        if(item!=nullptr){
            if(item->path()!=dir){
                item = item->findChild(dir);
            }
        }
        if(response->status()){
            auto list = response->parseList();
            if(item!=nullptr){
                //clear all children
                item->setExpanded(true);
                item->setLoading(false);
                model->refreshItems(list,item);
            }
            ServerRefreshData data{command,response->id,dir,list};
            Publisher::getInstance()->post(Type::M_NOTIFY_REFRESH_LIST,&data);
        }else{
            if(item!=nullptr){
                item->setLoading(false);
                model->changeItem(item);
            }
            wToast::showText(response->errorMsg);
        }
        this->output(response);
    }
    if(response!=nullptr){
        delete response;
    }
}

void ServerManagePane::onMessage(const QString& message,int result){
    wToast::showText(message);
}

void ServerManagePane::onTreeItemExpanded(const QModelIndex& index){
    auto item = static_cast<ServerManageModelItem*>(index.internalPointer());
    if(item!=nullptr){
        ServerManageModelItem::Type type = item->type();
        if(type==ServerManageModelItem::Server){
            if(item->expanded()==false){
                //init link
                this->connectServer(item->sid(),item->path());
                item->setExpanded(true);
                item->setLoading(true);
                static_cast<ServerManageModel*>(ui->treeView->model())->changeItem(item);
            }
        }else if(type==ServerManageModelItem::Folder){
            if(item->expanded()==false){
                long long sid = item->sid();
                this->cdDir(sid,item->path());
                item->setExpanded(true);
                item->setLoading(true);
                static_cast<ServerManageModel*>(ui->treeView->model())->changeItem(item);
            }
        }
    }
}

void ServerManagePane::onTreeItemDClicked(const QModelIndex& index){

}

void ServerManagePane::onActionTriggered(){
    QObject* sender = this->sender();
    auto model = ui->treeView->selectionModel();
    if(sender==ui->actionRefresh){
        QModelIndex index = model->currentIndex();
        if(index.isValid()){
            auto item = static_cast<ServerManageModelItem*>(index.internalPointer());
            auto type = item->type();
            if(type==ServerManageModelItem::Server || type==ServerManageModelItem::Folder ){
                if(item->expanded()){
                    long long sid = item->sid();
                    item->setLoading(true);
                    static_cast<ServerManageModel*>(ui->treeView->model())->changeItem(item);
                    this->cdDir(sid,item->path(),true);
                }else{
                    //connect server
                    ui->treeView->expand(index);
                }
            }
        }
    }else if(sender==ui->actionDock_To_Client){
        QModelIndex index = model->currentIndex();
        if(index.isValid()){
            auto item = static_cast<ServerManageModelItem*>(index.internalPointer());
            long long siteid = item->sid();
            auto model = static_cast<ServerManageModel*>(ui->treeView->model());
            auto site = model->find(siteid,false);
            auto workbench = this->container()->workbench();
            auto pane = new ServerClientPane(workbench,item->sid());
            pane->setRootPath(site->path());
            pane->setCurrentPath(item->path(),true);
            pane->setWindowTitle(site->name());
            auto manager = workbench->manager();
            manager->createPane(pane,DockingPaneManager::Center,true);
        }
    }else if(sender==ui->actionOpen){

    }else if(sender==ui->actionDownload){
        QModelIndexList indexlist = model->selectedRows();
        auto instance = Publisher::getInstance();
        instance->post(Type::M_OPEN_FILE_TRANSFTER);//open file transfer
        for(auto one:indexlist){
            auto item = static_cast<ServerManageModelItem*>(one.internalPointer());
            auto type = item->type();
            if(type == ServerManageModelItem::File || type==ServerManageModelItem::Folder || type==ServerManageModelItem::Server){
                //M_DOWNLOAD
                long long pid = item->pid();
                if(pid==0){
                    continue;
                }
                auto d = item->data();
                long long filesize = 0l;
                if(d!=nullptr){
                    filesize = d->size;
                }
                DownloadData data{pid,item->sid(),filesize,type==ServerManageModelItem::File,item->path(),{}};
                instance->post(Type::M_DOWNLOAD,&data);
            }
        }
    }else if(sender==ui->actionDownload_To){
        QModelIndexList indexlist = model->selectedRows();
        QList<DownloadData> list;
        QStringList stringlist;
        for(auto one:indexlist){
            auto item = static_cast<ServerManageModelItem*>(one.internalPointer());
            auto type = item->type();
            if(type == ServerManageModelItem::File || type==ServerManageModelItem::Folder || type==ServerManageModelItem::Server){
                stringlist << item->path();

                auto d = item->data();
                long long filesize = 0l;
                if(d!=nullptr){
                    filesize = d->size;
                }
                list.push_back({item->pid(),item->sid(),filesize,type==ServerManageModelItem::File,item->path(),{}});
            }
        }
        if(stringlist.size()>0){
            const QString local = QFileDialog::getExistingDirectory(this, tr("Select Download To Folder"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            if(!local.isEmpty()){
                if(MessageDialog::confirm(this,tr("Are you want to download \n\"%1\" \nfiles to \"%2\"?").arg(stringlist.join("\n")).arg(local))==QMessageBox::Yes){
                    auto instance = Publisher::getInstance();
                    instance->post(Type::M_OPEN_FILE_TRANSFTER);//open file transfer
                    auto iter = list.begin();
                    while(iter!=list.end()){
                        QString remote = (*iter).remote;
                        if((*iter).is_file==false && remote.endsWith("/")){
                            remote = remote.left(remote.size() - 1);
                        }
                        QFileInfo fi(remote);
                        (*iter).local = local+"/"+fi.fileName();

                        instance->post(Type::M_DOWNLOAD,&(*iter));
                        iter++;
                    }
                }
            }
        }

    }else if(sender == ui->actionPermissions){

        QModelIndexList indexlist = model->selectedRows();
        int size = 0;
        int mode = -1;
        QString filename;
        for(auto one:indexlist){
            auto item = static_cast<ServerManageModelItem*>(one.internalPointer());

            auto type = item->type();
            if(type==ServerManageModelItem::File || type==ServerManageModelItem::Folder){
                size += 1;
                if(filename.isEmpty()){
                    filename = item->name();
                    mode = PermissionDialog::format(item->data()->permission);
                }
            }
        }
        if(mode<0){
            mode = 0;
        }
        if(size>0){
            auto dialog = new PermissionDialog(size==1?mode:0,this);
            dialog->setFileInfo(filename,size);
            dialog->setModal(true);
            if(dialog->exec()==QDialog::Accepted){
                this->chmod(dialog->mode(),dialog->applyChildren());
                delete dialog;
            }
        }
    }else if(sender==ui->actionRename){
        QModelIndex index = model->currentIndex();
        auto item = static_cast<ServerManageModelItem*>(index.internalPointer());
        if(item->type()==ServerManageModelItem::Folder || item->type()==ServerManageModelItem::File){
            ui->treeView->edit(index);
        }
    }else if(sender==ui->actionDelete){
        QModelIndexList list = model->selectedRows();
        QMap<long long,QStringList> data;
        for(auto one:list){
            auto item = static_cast<ServerManageModelItem*>(one.internalPointer());
            if(item->type()==ServerManageModelItem::File || item->type()==ServerManageModelItem::Folder){
                long long sid = item->sid();
                if(!data.contains(sid)){
                    data.insert(sid,{item->path()});
                }else{
                    data[sid].push_back(item->path());
                }
            }
        }
        if(data.size()>0){
            if(MessageDialog::confirm(this,tr("Are you sure you want to delete the selected file?"))==QMessageBox::Yes){
                auto iter = data.begin();
                while(iter!=data.end()){
                    this->deleteFiles(iter.key(),iter.value());
                    iter++;
                }
            }
        }
    }else if(sender==ui->actionNew_Folder){
         QModelIndex index = model->currentIndex();
         auto item = static_cast<ServerManageModelItem*>(index.internalPointer());
         if(item->type()==ServerManageModelItem::Server || item->type()==ServerManageModelItem::Folder){
            //new folder dialog
            auto dialog = NewFolderDialog::open(item->sid(),item->path(),this);
            connect(dialog,&NewFolderDialog::submit,this,&ServerManagePane::onNewFolder);
         }
    }else if(sender==ui->actionConnect_Server){
        SiteQuickManagerDialog::open(this);
    }
}

void ServerManagePane::onNewFolder(long long id,const QString& path,const QString& name){
    auto req = NetworkManager::getInstance()->request(id);
    if(req!=nullptr){
         auto thread = new ServerRequestThread(req,ServerRequestThread::NewFolder,new QString(path+name));
         connect(thread,&ServerRequestThread::finished,thread,&ServerRequestThread::deleteLater);
         connect(thread,&ServerRequestThread::resultReady, this, &ServerManagePane::onNetworkResponse);
         connect(thread,&ServerRequestThread::output, this, &ServerManagePane::onOutput);
         this->addThread(thread);
         thread->start();
    }
}

void ServerManagePane::onRename(ServerManageModelItem* item,const QString& newName){
    auto id = item->sid();
    auto req = NetworkManager::getInstance()->request(id);
    if(req!=nullptr){
         const QString src = item->path();
         const QString parent = item->parent()->path();
         const QString dest = parent + newName;
         auto data = new RenameData{item->type(),src,dest,parent};

         auto thread = new ServerRequestThread(req,ServerRequestThread::Rename,data);
         connect(thread,&ServerRequestThread::finished,thread,&ServerRequestThread::deleteLater);
         connect(thread,&ServerRequestThread::resultReady, this, &ServerManagePane::onNetworkResponse);
         connect(thread,&ServerRequestThread::output, this, &ServerManagePane::onOutput);
         this->addThread(thread);
         thread->start();
    }
}


void ServerManagePane::chmod(int mode,bool apply_children){
    auto indexlist = ui->treeView->selectionModel()->selectedRows();
    QMap<long long,ChmodData*> data;
    for(auto one:indexlist){
         auto item = static_cast<ServerManageModelItem*>(one.internalPointer());
         auto type = item->type();
         if(type==ServerManageModelItem::File||type==ServerManageModelItem::Folder){
            long long sid = item->sid();
            if(!data.contains(sid)){
                data.insert(sid,new ChmodData{mode,apply_children});
            }
            data[sid]->list << item->path();
         }
    }

    QMap<long long,ChmodData*>::iterator iter = data.begin();
    while(iter!=data.end()){
         auto id = iter.key();
         auto req = NetworkManager::getInstance()->request(id);
         if(req!=nullptr){
            auto thread = new ServerRequestThread(req,ServerRequestThread::Chmod,iter.value());
            connect(thread,&ServerRequestThread::finished,thread,&ServerRequestThread::deleteLater);
            connect(thread,&ServerRequestThread::resultReady, this, &ServerManagePane::onNetworkResponse);
            connect(thread,&ServerRequestThread::message, this, &ServerManagePane::onMessage);
            connect(thread,&ServerRequestThread::output, this, &ServerManagePane::onOutput);
            this->addThread(thread);
            thread->start();
         }
         iter++;
    }

}

void ServerManagePane::refresh(ServerManageModelItem* item){
    auto type = item->type();
    if(type==ServerManageModelItem::Folder || type==ServerManageModelItem::Server){
        long long sid = item->sid();
        item->setLoading(true);
        static_cast<ServerManageModel*>(ui->treeView->model())->changeItem(item);
        this->cdDir(sid,item->path(),true);
    }
}

void ServerManagePane::output(NetworkResponse* response){
    if(response){
        if(response->status()){
            this->onOutput(response->command + "\n"+response->header,Type::Ok);
        }else{
            /*QJsonObject json = {
                {"level",Type::Error},
                {"source",PANE_TITLE},
                {"content",response->command + "\n"+response->errorMsg}
            };
            Publisher::getInstance()->post(Type::M_OUTPUT,json);*/

            this->onOutput(response->command + "\n"+response->errorMsg,Type::Error);
        }
    }
}

void ServerManagePane::onThreadFinished(){
    auto sender = static_cast<ServerRequestThread*>(this->sender());
    if(d->list.contains(sender)){
         d->list.removeOne(sender);
    }
}

void ServerManagePane::onOutput(const QString& message,int status){
    QJsonObject json = {
        {"level",status},
        {"source",PANE_TITLE},
        {"content",message}
    };
    Publisher::getInstance()->post(Type::M_OUTPUT,json);
}

void ServerManagePane::onDropUpload(const QMimeData* data){
    auto instance = Publisher::getInstance();
    QString paneGroup = FileTransferPane::PANE_GROUP;
    instance->post(Type::M_OPEN_PANE,&paneGroup);

    QList<UploadData> filelist;
    QString destination;
    if(data->hasFormat(MM_UPLOAD)){
        auto pos = ui->treeView->mapFromGlobal(QCursor::pos());
        auto index = ui->treeView->indexAt(pos);
        QByteArray b = data->data(MM_UPLOAD);
        QDataStream out(&b,QIODevice::ReadOnly);
        qint64 ptr;
        out >> ptr;
        auto list = (QStringList*)(ptr);
        if(index.isValid()){
            auto item = static_cast<ServerManageModelItem*>(index.internalPointer());
            if(item){
                auto type = item->type();
                if(type==ServerManageModelItem::Server || type==ServerManageModelItem::Folder){
                    destination = item->path();
                    if(!destination.endsWith("/")){
                        destination += "/";
                    }
                    int pid = item->pid();
                    int siteid = item->sid();

                    for(auto one:*list){
                        QFileInfo fi(one);
                        auto data = UploadData{pid,siteid,fi.isFile(),fi.absoluteFilePath(),destination+fi.fileName()};
                        filelist<<data;
                    }
                }
            }
        }
        delete list;
    }else if(data->hasUrls()){
        auto pos = ui->treeView->mapFromGlobal(QCursor::pos());
        auto index = ui->treeView->indexAt(pos);
        if(index.isValid()){
            auto item = static_cast<ServerManageModelItem*>(index.internalPointer());
            if(item){
                auto type = item->type();
                if(type==ServerManageModelItem::Server || type==ServerManageModelItem::Folder){
                    QList<QUrl> urls = data->urls();
                    destination = item->path();
                    if(!destination.endsWith("/")){
                        destination += "/";
                    }
                    int pid = item->pid();
                    int siteid = item->sid();
                    for(auto url:urls){
                        QFileInfo fi(url.toLocalFile());
                        auto data = UploadData{pid,siteid,fi.isFile(),fi.absoluteFilePath(),destination+fi.fileName()};
                        filelist<<data;
                    }
                }
            }
        }
    }
    if(filelist.size()>0)
        if(MessageDialog::confirm(this,tr("Upload Confirm"),tr("Are you want to upload files to \n\"%1\"").arg(destination))==QMessageBox::Ok){
            for(auto one:filelist){
                instance->post(Type::M_UPLOAD,&one);
            }
        }

}

ServerManagePane* ServerManagePane::open(DockingPaneManager* dockingManager,bool active){
    if(instance==nullptr){
        instance = new ServerManagePane(dockingManager->widget());
        DockingPaneLayoutItemInfo* item = dockingManager->createPane(instance,DockingPaneManager::S_Right,active);
        item->setManualSize(300);
    }
    return instance;

}

ServerManagePane* ServerManagePane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    if(instance==nullptr){
        instance = new ServerManagePane(dockingManager->widget());
        return instance;
    }
    return nullptr;
}
}
