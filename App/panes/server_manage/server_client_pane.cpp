#include "server_client_pane.h"
#include "ui_server_client_pane.h"
#include "network/network_manager.h"
#include "network/network_request.h"
#include "network/network_response.h"
#include "core/event_bus/event.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event_data.h"
#include "storage/site_storage.h"
#include <docking_pane_manager.h>
#include <docking_pane_container_tabbar.h>
#include "panes/file_transfer/file_transfer_pane.h"
#include "interface/remote_file_item_model.h"
#include "components/message_dialog.h"
#include "server_request_thread.h"
#include "new_folder_dialog.h"
#include "permission_dialog.h"
#include "common.h"
#include <w_toast.h>
#include <docking_pane_container.h>
#include <storage/site_storage.h>
#include <QTimer>
#include <QMenu>
#include <QStyledItemDelegate>
#include <QFileDialog>

namespace ady{

const QString ServerClientPane::PANE_ID = "ServerClient_%1_%2";
const QString ServerClientPane::PANE_GROUP = "ServerClient";


class ServerClientPanePrivate{
public:
    long long id=0l;
    long long pid=0l;
    bool showed=false;
    QString type;
    QString rootPath;
    QString currentPath;
    SiteRecord site;
    ServerRequestThread* thread = nullptr;
    RemoteFileItemModel* model = nullptr;
    QJsonObject toJson(){
        return {
            {"id",id},
            {"rootPath",rootPath},
            {"currentPath",currentPath}
        };
    }
};

ServerClientPane::ServerClientPane(QWidget* parent,long long id):
    DockingPane(parent),ui(new Ui::ServerClientPane)
{
    Subscriber::reg();


    SiteStorage storage;
    d = new ServerClientPanePrivate;
    d->site = storage.one(id);


    this->regMessageIds({Type::M_UPLOAD,Type::M_OPEN_PROJECT,Type::M_CLOSE_PROJECT_NOTIFY,Type::M_NOTIFY_REFRESH_LIST,Type::M_CLOSE_PROJECT_NOTIFY,
                         Type::M_SITE_UPDATED,Type::M_SITE_DELETED});
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(d->site.name);
    this->setStyleSheet("QListView,QTreeView{border:0;}"
                        "QListView::item{ margin: 20px;}");



    d->id = id;
    d->pid = d->site.pid;
    d->type = d->site.type;


    ui->treeView->header()->setSortIndicator(0,Qt::AscendingOrder);

    d->model = new RemoteFileItemModel(id,this);
    connect(d->model,&RemoteFileItemModel::cellEditing,this,&ServerClientPane::onRename);


    ui->treeView->setModel(d->model);
    ui->treeView->setColumnWidth(RemoteFileItemModel::Name,220);
    ui->treeView->setColumnWidth(RemoteFileItemModel::Size,100);
    ui->treeView->setColumnWidth(RemoteFileItemModel::ModifyTime,160);
    ui->treeView->addMimeType(MM_UPLOAD);
    ui->treeView->setSupportDropFile(true);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setEditTriggers(QAbstractItemView::EditKeyPressed);

    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->listView->setBatchSize(20);
    ui->listView->setSpacing(20);
    ui->listView->setGridSize({140,180});
    ui->listView->setIconSize({125,125});



    connect(ui->treeView,&QTreeView::activated,this,&ServerClientPane::onTreeItemDClicked);
    connect(ui->treeView,&TreeView::dropFinished,this,&ServerClientPane::onDropUpload);
    connect(ui->treeView,&QTreeView::customContextMenuRequested,this,&ServerClientPane::onContextMenu);

    //tree view sorting
    connect(ui->treeView->header(),&QHeaderView::sortIndicatorChanged,d->model,&RemoteFileItemModel::sortList);


    //listview
    ui->listView->setModel(d->model);
    connect(ui->listView,&QListView::activated,this,&ServerClientPane::onTreeItemDClicked);
    //connect(ui->listView,&QListView::dropFinished,this,&ServerClientPane::onDropUpload);
    connect(ui->listView,&QListView::customContextMenuRequested,this,&ServerClientPane::onContextMenu);


    connect(ui->actionBack,&QAction::triggered,this,&ServerClientPane::onActionTriggered);
    connect(ui->actionRefresh,&QAction::triggered,this,&ServerClientPane::onActionTriggered);
    connect(ui->actionGridView,&QAction::triggered,this,&ServerClientPane::onActionTriggered);
    connect(ui->actionListView,&QAction::triggered,this,&ServerClientPane::onActionTriggered);

    connect(ui->actionDownload,&QAction::triggered,this,&ServerClientPane::onActionTriggered);
    connect(ui->actionRename,&QAction::triggered,this,&ServerClientPane::onActionTriggered);
    connect(ui->actionPermissions,&QAction::triggered,this,&ServerClientPane::onActionTriggered);
    connect(ui->actionNew_Folder,&QAction::triggered,this,&ServerClientPane::onActionTriggered);
    connect(ui->actionDelete,&QAction::triggered,this,&ServerClientPane::onActionTriggered);

}


ServerClientPane::~ServerClientPane(){
    Subscriber::unReg();

    if(d->thread!=nullptr){
        d->thread->requestInterruption();
        d->thread->quit();
    }
    delete ui;
    delete d;
}



void ServerClientPane::initView(){

}



QString ServerClientPane::id(){

    return PANE_ID.arg(d->type).arg(d->id);
}


QString ServerClientPane::group(){
    return PANE_GROUP;
}


bool ServerClientPane::onReceive(Event* e){
    if(e->id()==Type::M_NOTIFY_REFRESH_LIST){
        auto data = e->toJsonOf<ServerRefreshData>().toObject();
        long long id = data.find("id")->toInt(0);
        const QString path = data.find("path")->toString();
        if(id==d->id && path == d->currentPath){
            //this->reload();
            auto array = data.find("list")->toArray();
            QList<FileItem> list;
            for(auto one:array){
                list<<FileItem::fromJson(one.toObject());
            }
            auto model = static_cast<RemoteFileItemModel*>(ui->treeView->model());
            FileItem item;
            item.type = FileItem::Dir;
            item.name = "..";
            item.path = path.mid(0,path.lastIndexOf("/",-2) + 1);
            if(path==d->rootPath){
                item.enabled = false;
            }
            list.insert(0,item);
            model->setList(list);
            return true;
        }
    }else if(e->id()==Type::M_CLOSE_PROJECT_NOTIFY){
        auto data = e->toJsonOf<CloseProjectData>().toObject();
        long long id = data.find("id")->toInt(0);
        if(id!=0 && id==d->pid){
            //close pane
            auto container = this->container();
            if(container!=nullptr){
                container->closePane(this,true);
            }
        }
    }else if(e->id()==Type::M_SITE_UPDATED){
        auto site = e->toJsonOf<SiteRecord>().toObject();
        auto id = site.find("id")->toInt(0);
        if(id==d->id){
            //update title
            auto container = this->container();
            //container->setT
            if(container!=nullptr){
                int i = container->indexOf(this);
                if(i>=0){
                    auto tabBar = container->tabBar();
                    tabBar->setTabText(i,site.find("name")->toString());
                }
            }
        }
    }else if(e->id()==Type::M_SITE_DELETED){
        auto site = e->toJsonOf<SiteRecord>().toObject();
        auto id = site.find("id")->toInt(0);
        if(id==d->id){
            //close pane
            auto container = this->container();
            container->closePane(this);
        }
    }
    return false;
}

QJsonObject ServerClientPane::toJson(){
    return {
        {"id",this->id()},
        {"group",this->group()},
        {"data",d->toJson()}
    };
}

bool ServerClientPane::isThreadRunning(){
    return d->thread!=nullptr;
}

void ServerClientPane::setRootPath(const QString& path){
    d->rootPath = path;
}

QString ServerClientPane::rootPath(){
    return d->rootPath;
}

void ServerClientPane::setCurrentPath(const QString& path,bool request){
    d->currentPath = path;
    qDebug()<<"setCurrentPath"<<path;
    ui->lineEdit->setText(d->currentPath);
    if(request){
        //this->reload();
    }
}

QString ServerClientPane::currentPath(){
    return d->currentPath;
}

void ServerClientPane::reload(){
    qDebug()<<"current path"<<d->currentPath<<d->rootPath;
    if(d->currentPath==d->rootPath){
        this->connectServer(d->id,d->currentPath);
    }else{

        this->cdDir(d->id,d->currentPath,true);
    }
}

void ServerClientPane::output(NetworkResponse* response){
    if(response){
        if(response->status()){

            this->onOutput(response->command + "\n"+response->header,1);
        }else{

            this->onOutput(response->command + "\n"+(response->errorInfo()),3);
        }
    }
}


ServerClientPane* ServerClientPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){

    long long id = data.find("id")->toInt(0);
    if(id==0){
        return nullptr;
    }
    auto pane = new ServerClientPane(dockingManager->widget(),id);
    pane->setRootPath(data.find("rootPath")->toString());
    pane->setCurrentPath(data.find("currentPath")->toString());

    return pane;

}

void ServerClientPane::showEvent(QShowEvent* e){
    DockingPane::showEvent(e);
    if(d->showed==false){
        if(d->site.pid==0){
            //init quick connect
            Publisher::getInstance()->post(Type::M_SITE_ADDED,&d->site);
            NetworkManager::getInstance()->initRequest(d->id,d->type);
        }
        d->showed = true;
        this->reload();
    }
}

void ServerClientPane::onContextMenu(const QPoint& pos){
    if(this->isThreadRunning()){
        return ;
    }
    QMenu contextMenu(this);

    auto view = static_cast<QAbstractItemView*>(this->sender());
    auto model = view->selectionModel();
    QModelIndex index = model->currentIndex();
    //int total = model->model()->rowCount();
    if(index.isValid()){
        //auto item =
        //auto item = (static_cast<RemoteFileItemModel*>(ui->treeView->model()))->getItem(index.row());
        contextMenu.addAction(ui->actionDownload);
        contextMenu.addSeparator();

        contextMenu.addAction(ui->actionRename);
        contextMenu.addAction(ui->actionPermissions);
        contextMenu.addSeparator();
        contextMenu.addAction(ui->actionNew_Folder);
        contextMenu.addAction(ui->actionDelete);
        contextMenu.exec(QCursor::pos());
    }
}

void ServerClientPane::connectServer(long long id,const QString& path){
    if(isThreadRunning()){
        return ;
    }
    auto req = NetworkManager::getInstance()->request(id);
    if(req!=nullptr){
        ui->widget->start();
        d->thread = new ServerRequestThread(req,ServerRequestThread::Link,new QString(path));
        connect(d->thread,&ServerRequestThread::finished,this,&ServerClientPane::onThreadFinished);
        connect(d->thread,&ServerRequestThread::resultReady, this, &ServerClientPane::onNetworkResponse);
        //connect(d->thread,&ServerRequestThread::output, this, &ServerClientPane::onOutput);
        d->thread->start();
    }
}

void ServerClientPane::cdDir(long long id,const QString& path,bool refresh){
    if(isThreadRunning()){
        return ;
    }
    auto req = NetworkManager::getInstance()->request(id);
    if(req!=nullptr){
        if(req->isConnected()==false){
            this->connectServer(id,path);
            return ;
        }
        ui->widget->start();
        d->thread = new ServerRequestThread(req,refresh?ServerRequestThread::Refresh:ServerRequestThread::List,new QString(path));
        connect(d->thread,&ServerRequestThread::finished,this,&ServerClientPane::onThreadFinished);
        connect(d->thread,&ServerRequestThread::resultReady, this, &ServerClientPane::onNetworkResponse);
        d->thread->start();
    }
}

void ServerClientPane::deleteFiles(long long id,const QStringList& list){
    if(isThreadRunning()){
        return ;
    }
    auto req = NetworkManager::getInstance()->request(id);
    if(req!=nullptr){
        ui->widget->start();
        d->thread = new ServerRequestThread(req,ServerRequestThread::Delete,new QStringList(list));
        connect(d->thread,&ServerRequestThread::finished,this,&ServerClientPane::onThreadFinished);
        connect(d->thread,&ServerRequestThread::resultReady, this, &ServerClientPane::onNetworkResponse);
        connect(d->thread,&ServerRequestThread::message, this, &ServerClientPane::onMessage);
        connect(d->thread,&ServerRequestThread::output, this, &ServerClientPane::onOutput);
        d->thread->start();
    }
}

void ServerClientPane::onNetworkResponse(NetworkResponse* response,int command,int result){
    if(result==ServerRequestThread::Unsppport){
        wToast::showText(tr("This operation is not supported"));
        return ;
    }
    //auto model = static_cast<RemoteFileItemModel*>(ui->treeView->model());
    if(command==ServerRequestThread::Link || command==ServerRequestThread::List || command==ServerRequestThread::Refresh || command==ServerRequestThread::NewFolder || command==ServerRequestThread::Delete){
        if(response->status()){
            //list default dir
            auto list = response->parseList();
            auto dir = response->params["dir"].toString();
            ServerRefreshData data{command,d->id,dir,list};//

            FileItem item;
            item.type = FileItem::Dir;
            item.name = "..";
            item.path = dir.mid(0,dir.lastIndexOf("/",-2) + 1);
            if(dir==d->rootPath){
                item.enabled = false;
            }
            list.insert(0,item);

            d->model->setList(list);
            ui->lineEdit->setText(dir);
            d->currentPath = dir;

            Publisher::getInstance()->post(Type::M_NOTIFY_REFRESH_TREE,&data);
        }else{

            wToast::showText(response->errorInfo());
        }
    }else if(command==ServerRequestThread::Rename || command==ServerRequestThread::Chmod){
        if(response->status()){
            //list default dir
            auto list = response->parseList();
            auto dir = response->params["dir"].toString();

            FileItem item;
            item.type = FileItem::Dir;
            item.name = "..";
            item.path = dir.mid(0,dir.lastIndexOf("/",-2) + 1);
            if(dir==d->rootPath){
                item.enabled = false;
            }
            list.insert(0,item);
            d->model->setList(list);
            ui->lineEdit->setText(dir);
            d->currentPath = dir;

            ServerRefreshData data{command,d->id,d->currentPath};
            Publisher::getInstance()->post(Type::M_NOTIFY_REFRESH_TREE,&data);

        }else{
            wToast::showText(response->errorMsg);
        }
    }
    if(response!=nullptr){
        //response->debug();
        this->output(response);
        delete response;
    }
}

void ServerClientPane::onMessage(const QString& message,int result){
    wToast::showText(message);
}



void ServerClientPane::onTreeItemDClicked(const QModelIndex& index){
    auto item = d->model->getItem(index.row());
    if(item.enabled && item.type==FileItem::Folder)
        this->cdDir(d->id,item.path,false);
}

void ServerClientPane::onDropUpload(const QMimeData* data){
    auto instance = Publisher::getInstance();
    QString paneGroup = FileTransferPane::PANE_GROUP;
    instance->post(Type::M_OPEN_PANE,&paneGroup);

    if(data->hasFormat(MM_UPLOAD)){
        QByteArray b = data->data(MM_UPLOAD);
        QDataStream out(&b,QIODevice::ReadOnly);
        qint64 ptr;
        out >> ptr;
        auto list = (QStringList*)(ptr);
        QString destination = d->currentPath;
        if(!destination.endsWith("/")){
            destination += "/";
        }
        qDebug()<<"destination"<<destination;
        for(auto one:*list){
            QFileInfo fi(one);
            auto data = UploadData{d->pid,d->id,fi.isFile(),fi.absoluteFilePath(),destination+fi.fileName()};
            instance->post(Type::M_UPLOAD,&data);
        }
        delete list;
    }else if(data->hasUrls()){
        QList<QUrl> urls = data->urls();
        QString destination = d->currentPath;
        if(!destination.endsWith("/")){
            destination += "/";
        }
        for(auto url:urls){
            QFileInfo fi(url.toLocalFile());
            auto data = UploadData{d->pid,d->id,fi.isFile(),fi.absoluteFilePath(),destination+fi.fileName()};
            instance->post(Type::M_UPLOAD,&data);
        }
    }
}

void ServerClientPane::onActionTriggered(){
    if(this->isThreadRunning()){
        return ;
    }
    QObject* sender = this->sender();
    //auto model = ui->treeView->selectionModel();
    auto model = ui->stackedWidget->currentIndex()==0?ui->treeView->selectionModel():ui->listView->selectionModel();
    if(sender==ui->actionRefresh){
        this->reload();
    }else if(sender==ui->actionBack){
        if(d->rootPath!=d->currentPath){
            QString path = d->currentPath.mid(0,d->currentPath.lastIndexOf("/",-2)+1);
            this->cdDir(d->id,path);
        }
    }else if(sender==ui->actionGridView){
        d->model->setMode(RemoteFileItemModel::Grid);
        ui->actionGridView->setChecked(true);
        ui->actionListView->setChecked(false);
        ui->stackedWidget->setCurrentIndex(1);
    }else if(sender==ui->actionListView){
        d->model->setMode(RemoteFileItemModel::List);
        ui->actionListView->setChecked(true);
        ui->actionGridView->setChecked(false);
        ui->stackedWidget->setCurrentIndex(0);
    }else if(sender==ui->actionDelete){
        //QModelIndexList indexlist = model->selectedRows();
        QModelIndexList indexlist;
        if(ui->stackedWidget->currentIndex()==0){
            indexlist = model->selectedRows();
        }else{
            indexlist = model->selectedIndexes();
        }
        QMap<long long,QStringList> data;
        //auto m = static_cast<RemoteFileItemModel*>(ui->treeView->model());
        QStringList list;
        for(auto one:indexlist){
            auto item = d->model->getItem(one.row());
            if(item.name!=".."){
                list<<item.path;
            }
        }
        if(list.size()>0){
            if(MessageDialog::confirm(this,tr("Are you sure you want to delete the selected file?"))==QMessageBox::Yes){
                this->deleteFiles(d->id,list);
            }
        }
    }else if(sender==ui->actionDownload){
        //QModelIndexList indexlist = model->selectedRows();
        QModelIndexList indexlist;
        if(ui->stackedWidget->currentIndex()==0){
            indexlist = model->selectedRows();
        }else{
            indexlist = model->selectedIndexes();
        }
        QString local;

        if(d->pid==0l){
            local = QFileDialog::getExistingDirectory(this, tr("Select Download To Folder"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            if(local.isEmpty()){
                return ;
            }
        }
        auto instance = Publisher::getInstance();
        instance->post(Type::M_OPEN_FILE_TRANSFTER);//open file transfer
        //auto m = static_cast<RemoteFileItemModel*>(ui->treeView->model());
        for(auto one:indexlist){
            auto item = d->model->getItem(one.row());
            if(item.type==FileItem::File || item.type==FileItem::Folder){
                long long filesize = item.size;
                const QString destination = d->pid==0l?(local+"/"+item.name):"";
                DownloadData data{0,d->id,filesize,item.type==FileItem::File,item.path,destination};
                instance->post(Type::M_DOWNLOAD,&data);
            }
        }
    }else if(sender==ui->actionPermissions){
        QModelIndexList indexlist;
        if(ui->stackedWidget->currentIndex()==0){
            indexlist = model->selectedRows();
        }else{
            indexlist = model->selectedIndexes();
        }
        int size = 0;
        int mode = -1;
        QString filename;
        QStringList list;
        //auto m = static_cast<RemoteFileItemModel*>(ui->treeView->model());
        for(auto one:indexlist){
            auto item = d->model->getItem(one.row());
            if(item.name!=".."){
                size+=1;
                list<<item.path;
            }
            if(filename.isEmpty()){
                filename = item.name;
                mode = PermissionDialog::format(item.permission);
            }
        }

        if(mode<0){
            mode = 0;
        }
        if(list.size()>0){
            auto dialog = new PermissionDialog(size==1?mode:0,this);
            dialog->setFileInfo(filename,size);
            dialog->setModal(true);
            if(dialog->exec()==QDialog::Accepted){
                int mode = dialog->mode();
                bool applyChildren = dialog->applyChildren();
                delete  dialog;
                auto data = new ChmodData{mode,applyChildren};
                data->list = list;

                auto req = NetworkManager::getInstance()->request(d->id);
                if(req!=nullptr){
                    ui->widget->start();
                    d->thread = new ServerRequestThread(req,ServerRequestThread::Chmod,data);
                    connect(d->thread,&ServerRequestThread::finished,this,&ServerClientPane::onThreadFinished);
                    connect(d->thread,&ServerRequestThread::resultReady, this, &ServerClientPane::onNetworkResponse);
                    connect(d->thread,&ServerRequestThread::message, this, &ServerClientPane::onMessage);
                    connect(d->thread,&ServerRequestThread::output, this, &ServerClientPane::onOutput);
                    d->thread->start();
                }else{
                    delete data;
                }
            }
        }
    }else if(sender==ui->actionRename){
        if(ui->stackedWidget->currentIndex()==0){
            QModelIndex index = ui->treeView->currentIndex();
            ui->treeView->edit(index);
        }else{
            QModelIndex index = ui->listView->currentIndex();
            ui->listView->edit(index);
        }
    }else if(sender==ui->actionNew_Folder){
        auto dialog = NewFolderDialog::open(d->id,d->currentPath,this);
        connect(dialog,&NewFolderDialog::submit,this,&ServerClientPane::onNewFolder);
    }
}

void ServerClientPane::onNewFolder(long long id,const QString& path,const QString& name){
    if(isThreadRunning()){
        return ;
    }
    auto req = NetworkManager::getInstance()->request(id);
    if(req!=nullptr){
        ui->widget->start();
        d->thread = new ServerRequestThread(req,ServerRequestThread::NewFolder,new QString(path+name));
        connect(d->thread,&ServerRequestThread::finished,this,&ServerClientPane::onThreadFinished);
        connect(d->thread,&ServerRequestThread::resultReady, this, &ServerClientPane::onNetworkResponse);
        connect(d->thread,&ServerRequestThread::output, this, &ServerClientPane::onOutput);
        d->thread->start();
    }
}

void ServerClientPane::onRename(const QModelIndex index,const QString& newName){
    //auto id = item->sid();
    if(isThreadRunning()){
        return ;
    }
    auto req = NetworkManager::getInstance()->request(d->id);
    if(req!=nullptr){
        ui->widget->start();
        //auto model = static_cast<RemoteFileItemModel*>(ui->treeView->model());
        auto item = d->model->getItem(index.row());

        const QString src = item.path;
        QString parent = src;
        if(item.type==FileItem::Folder && parent.endsWith("/")){
            parent = parent.left(parent.size() - 1);
        }
        int offset = parent.lastIndexOf("/");
        if(offset>-1){
            parent = parent.left(offset);
        }
        if(!parent.endsWith("/")){
            parent += "/";
        }
        const QString dest = parent + newName;
        auto data = new RenameData{item.type,src,dest,parent};

        d->thread = new ServerRequestThread(req,ServerRequestThread::Rename,data);
        connect(d->thread,&ServerRequestThread::finished,this,&ServerClientPane::onThreadFinished);
        connect(d->thread,&ServerRequestThread::resultReady, this, &ServerClientPane::onNetworkResponse);
        connect(d->thread,&ServerRequestThread::output, this, &ServerClientPane::onOutput);
        d->thread->start();
    }
}

void ServerClientPane::onThreadFinished(){
    d->thread->deleteLater();
    d->thread = nullptr;
    ui->widget->stop();
}

void ServerClientPane::onOutput(const QString& message,int status){
    QJsonObject json = {
        {"level",status},
        {"source",this->windowTitle()},
        {"content",message}
    };
    Publisher::getInstance()->post(Type::M_OUTPUT,json);
}




}
