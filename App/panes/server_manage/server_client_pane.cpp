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
#include "docking_pane_manager.h"
#include "interface/remote_file_item_model.h"
#include "components/message_dialog.h"
#include "server_request_thread.h"
#include "new_folder_dialog.h"
#include "permission_dialog.h"
#include "common.h"
#include "w_toast.h"
#include <QTimer>
#include <QMenu>

namespace ady{

const QString ServerClientPane::PANE_ID = "ServerClient_%1_%2";
const QString ServerClientPane::PANE_GROUP = "ServerClient";

class ServerClientPanePrivate{
public:
    long long id=0l;
    bool showed=false;
    QString type;
    QString rootPath;
    QString currentPath;
    ServerRequestThread* thread = nullptr;
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
    auto r = storage.one(id);

    this->regMessageIds({Type::M_UPLOAD,Type::M_OPEN_PROJECT,Type::M_CLOSE_PROJECT_NOTIFY,Type::M_NOTIFY_REFRESH_LIST});
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(r.name);
    this->setStyleSheet("QToolBar{border:0px;}"
                        "QTreeView{border:0;background-color:#f5f5f5}"
                        ".ady--ServerClientPane>#widget{background-color:#EEEEF2}");


    d = new ServerClientPanePrivate;
    d->id = id;
    d->type = r.type;


    ui->treeView->header()->setSortIndicator(0,Qt::AscendingOrder);

    RemoteFileItemModel* model = new RemoteFileItemModel(id,ui->treeView);
    connect(model,&RemoteFileItemModel::cellEditing,this,&ServerClientPane::onRename);


    ui->treeView->setModel(model);
    ui->treeView->setColumnWidth(RemoteFileItemModel::Name,220);
    ui->treeView->setColumnWidth(RemoteFileItemModel::Size,100);
    ui->treeView->setColumnWidth(RemoteFileItemModel::ModifyTime,160);
    ui->treeView->addMimeType(MM_UPLOAD);
    ui->treeView->setSupportDropFile(true);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setEditTriggers(QAbstractItemView::EditKeyPressed);


    connect(ui->treeView,&QTreeView::activated,this,&ServerClientPane::onTreeItemDClicked);
    connect(ui->treeView,&TreeView::dropFinished,this,&ServerClientPane::onDropUpload);
    connect(ui->treeView,&QTreeView::customContextMenuRequested,this,&ServerClientPane::onContextMenu);

    //tree view sorting
    connect(ui->treeView->header(),&QHeaderView::sortIndicatorChanged,model,&RemoteFileItemModel::sortList);


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
    ui->lineEdit->setText(d->currentPath);
    if(request){
        //this->reload();
    }
}

QString ServerClientPane::currentPath(){
    return d->currentPath;
}

void ServerClientPane::reload(){
    if(d->currentPath==d->rootPath){
        this->connectServer(d->id,d->currentPath);
    }else{
        this->cdDir(d->id,d->currentPath,true);
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
        d->showed = true;
        QTimer::singleShot(0,[this]{
            this->reload();
        });
    }
}

void ServerClientPane::onContextMenu(const QPoint& pos){
    if(this->isThreadRunning()){
        return ;
    }
    QMenu contextMenu(this);
    auto model = ui->treeView->selectionModel();
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
        d->thread->start();
    }
}

void ServerClientPane::onNetworkResponse(NetworkResponse* response,int command,int result){
    if(result==ServerRequestThread::Unsppport){
        wToast::showText(tr("This operation is not supported"));
        return ;
    }
    auto model = static_cast<RemoteFileItemModel*>(ui->treeView->model());
    if(command==ServerRequestThread::Link || command==ServerRequestThread::List || command==ServerRequestThread::Refresh || command==ServerRequestThread::NewFolder || command==ServerRequestThread::Delete){
        if(response->status()){
            //list default dir
            auto list = response->parseList();
            auto dir = response->params["dir"].toString();
            ServerRefreshData data{d->id,dir,list};//

            FileItem item;
            item.type = FileItem::Dir;
            item.name = "..";
            item.path = dir.mid(0,dir.lastIndexOf("/",-2) + 1);
            if(dir==d->rootPath){
                item.enabled = false;
            }
            list.insert(0,item);

            model->setList(list);
            ui->lineEdit->setText(dir);
            d->currentPath = dir;

            Publisher::getInstance()->post(Type::M_NOTIFY_REFRESH_TREE,&data);
        }else{
            wToast::showText(response->errorMsg);
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
            model->setList(list);
            ui->lineEdit->setText(dir);
            d->currentPath = dir;

            ServerRefreshData data{d->id,d->currentPath};
            Publisher::getInstance()->post(Type::M_NOTIFY_REFRESH_TREE,&data);

        }else{
            wToast::showText(response->errorMsg);
        }
    }
    if(response!=nullptr){
        delete response;
    }
}

void ServerClientPane::onMessage(const QString& message,int result){
    wToast::showText(message);
}



void ServerClientPane::onTreeItemDClicked(const QModelIndex& index){
    auto model = static_cast<RemoteFileItemModel*>(ui->treeView->model());
    auto item = model->getItem(index.row());
    if(item.enabled)
        this->cdDir(d->id,item.path,false);
}

void ServerClientPane::onDropUpload(const QMimeData* data){
    if(data->hasFormat(MM_UPLOAD)){
        /*QByteArray b = data->data(MM_UPLOAD);
        QDataStream out(&b,QIODevice::ReadOnly);
        qint64 p;
        out >> p;
        QList<FileItem>* lists = (QList<FileItem>*)(p);

        QList<FileItem>::iterator iter = lists->begin();
        QString siteCurrentDir = this->dirRender->text();
        if(!siteCurrentDir.endsWith("/")){
            siteCurrentDir += "/";
        }
        QList<Task*> tasks ;
        while(iter!=lists->end()){
            Task* t = new Task;
            t->type = (*iter).type==FileItem::Type::File?0:1;//0=file 1=dir
            t->cmd = UPLOAD;
            t->local = (*iter).path;
            t->remote = siteCurrentDir + (*iter).name;
            t->filesize = (*iter).size;
            t->siteid = this->id;
            tasks.push_back(t);
            iter++;
        }
        delete lists;

        if(tasks.size()>0)
        {
            emit command(tasks);
        }*/
    }else if(data->hasUrls()){
        //drop system files
        /*QString siteCurrentDir = this->dirRender->text();
        if(!siteCurrentDir.endsWith("/")){
            siteCurrentDir += "/";
        }
        QList<QUrl> urls = data->urls();
        QList<Task*> tasks ;
        QList<QUrl>::iterator iter = urls.begin();
        while(iter!=urls.end()){
            QString path = (*iter).path().mid(1);
            QFileInfo fi(path);
            if(fi.exists()){
                Task* t = new Task;
                t->type = fi.isDir()==false?0:1;//0=file 1=dir
                t->cmd = UPLOAD;
                t->local = path;
                t->remote = siteCurrentDir + fi.fileName();
                if(t->type==0){
                    t->filesize = fi.size();
                }
                t->siteid = this->id;
                tasks.push_back(t);
            }
            iter++;
        }
        if(tasks.size()>0){
            emit command(tasks);
        }*/
    }
}

void ServerClientPane::onActionTriggered(){
    if(this->isThreadRunning()){
        return ;
    }
    QObject* sender = this->sender();
    auto model = ui->treeView->selectionModel();
    if(sender==ui->actionRefresh){
        this->reload();
    }else if(sender==ui->actionBack){
        if(d->rootPath!=d->currentPath){
            QString path = d->currentPath.mid(0,d->currentPath.lastIndexOf("/",-2)+1);
            this->cdDir(d->id,path);
        }
    }else if(sender==ui->actionGridView){

    }else if(sender==ui->actionListView){

    }else if(sender==ui->actionDelete){
        QModelIndexList indexlist = model->selectedRows();
        QMap<long long,QStringList> data;
        auto m = static_cast<RemoteFileItemModel*>(ui->treeView->model());
        QStringList list;
        for(auto one:indexlist){
            auto item = m->getItem(one.row());
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
        QModelIndexList indexlist = model->selectedRows();
        auto instance = Publisher::getInstance();
        instance->post(Type::M_OPEN_FILE_TRANSFTER);//open file transfer
        auto m = static_cast<RemoteFileItemModel*>(ui->treeView->model());
        for(auto one:indexlist){
            auto item = m->getItem(one.row());
            if(item.type==FileItem::File || item.type==FileItem::Folder){
                long long filesize = item.size;
                DownloadData data{0,d->id,filesize,item.type==FileItem::File,item.path,{}};
                instance->post(Type::M_DOWNLOAD,&data);
            }
        }
    }else if(sender==ui->actionPermissions){
        QModelIndexList indexlist = model->selectedRows();
        int size = 0;
        int mode = -1;
        QString filename;
        QStringList list;
        auto m = static_cast<RemoteFileItemModel*>(ui->treeView->model());
        for(auto one:indexlist){
            auto item = m->getItem(one.row());
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
                d->thread->start();
            }else{
                delete data;
            }
        }
    }else if(sender==ui->actionRename){
        QModelIndex index = ui->treeView->currentIndex();
        ui->treeView->edit(index);
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
        auto model = static_cast<RemoteFileItemModel*>(ui->treeView->model());
        auto item = model->getItem(index.row());

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
        d->thread->start();
    }
}

void ServerClientPane::onThreadFinished(){
    d->thread->deleteLater();
    d->thread = nullptr;
    ui->widget->stop();
}


}
