#include "server_manage_pane.h"
#include "ui_server_manage_pane.h"
#include "docking_pane_manager.h"
#include "docking_pane_layout_item_info.h"
#include "core/event_bus/event.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event_data.h"
#include "storage/project_storage.h"
#include "server_manage_model.h"
#include "network/network_manager.h"
#include "server_request_thread.h"
#include "new_folder_dialog.h"
#include "w_toast.h"
#include "components/message_dialog.h"

#include <QMenu>
#include <QThread>
#include <QDebug>
namespace ady{

ServerManagePane* ServerManagePane::instance = nullptr;

const QString ServerManagePane::PANE_ID = "ServerManager";
const QString ServerManagePane::PANE_GROUP = "ServerManager";


class ServerManagePanePrivate{
public:
    QList<ServerRequestThread*> list;
};

ServerManagePane::ServerManagePane(QWidget *parent) :
    DockingPane(parent),
    ui(new Ui::ServerManagePane)
{
    Subscriber::reg();
    this->regMessageIds({Type::M_UPLOAD,Type::M_OPEN_PROJECT,Type::M_CLOSE_PROJECT_NOTIFY});
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(tr("Server Manager"));
    this->setStyleSheet("QToolBar{border:0px;}"
                        "QTreeView{border:0;background-color:#f5f5f5}"
                        ".ady--ServerManagePane>#widget{background-color:#EEEEF2}");

    d = new ServerManagePanePrivate;

    auto model = new ServerManageModel(ui->treeView);

    connect(model,&ServerManageModel::rename,this,&ServerManagePane::onRename);

    ui->treeView->setModel(model);
    ui->treeView->setColumnWidth(ServerManageModel::Name,240);

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView,&QTreeView::customContextMenuRequested, this, &ServerManagePane::onContextMenu);
    connect(ui->treeView,&QTreeView::expanded,this,&ServerManagePane::onTreeItemExpanded);
    //connect(ui->treeView,&QTreeView::doubleClicked,this,&ServerManagePane::onTreeItemDClicked);


    connect(ui->actionRefresh,&QAction::triggered,this,&ServerManagePane::onActionTriggered);
    connect(ui->actionDock_To_Client,&QAction::triggered,this,&ServerManagePane::onActionTriggered);
    connect(ui->actionOpen,&QAction::triggered,this,&ServerManagePane::onActionTriggered);
    connect(ui->actionDownload,&QAction::triggered,this,&ServerManagePane::onActionTriggered);
    connect(ui->actionDownload_To,&QAction::triggered,this,&ServerManagePane::onActionTriggered);
    connect(ui->actionRename,&QAction::triggered,this,&ServerManagePane::onActionTriggered);
    connect(ui->actionDelete,&QAction::triggered,this,&ServerManagePane::onActionTriggered);
    connect(ui->actionNew_Folder,&QAction::triggered,this,&ServerManagePane::onActionTriggered);


    this->initView();
}

ServerManagePane::~ServerManagePane()
{
    Subscriber::unReg();
    for(auto one:d->list){
        one->interrupt();
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
            contextMenu.addAction(ui->actionOpen);
        }
    }
    contextMenu.addSeparator();
    contextMenu.addAction(ui->actionDownload);
    contextMenu.addAction(ui->actionDownload_To);
    contextMenu.addSeparator();
    contextMenu.addAction(ui->actionRename);
    contextMenu.addAction(ui->actionDelete);
    contextMenu.exec(QCursor::pos());
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
        this->addThread(thread);
        thread->start();
    }
}

void ServerManagePane::onNetworkResponse(NetworkResponse* response,int command,int result){
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
        }else{
            if(item!=nullptr){
                item->setLoading(false);
            }
            wToast::showText(response->errorMsg);
        }
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
        }else{
            if(item!=nullptr){
                item->setLoading(false);
            }
            wToast::showText(response->errorMsg);
        }
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
                    model->refreshItems(list,item);
                }
            }
        }else{
            if(item!=nullptr){
                item->setLoading(false);
            }
            wToast::showText(response->errorMsg);
        }
    }else if(command==ServerRequestThread::Rename){
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
            if(result==ServerRequestThread::Unsppport){
                wToast::showText(tr("This operation is not supported"));
            }
        }else{
            if(item!=nullptr){
                item->setLoading(false);
            }
            wToast::showText(response->errorMsg);
        }
    }
    //response->debug();
    if(response!=nullptr){
        delete response;
    }
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
            }
        }else if(type==ServerManageModelItem::Folder){
            if(item->expanded()==false){
                long long sid = item->sid();
                this->cdDir(sid,item->path());
                item->setExpanded(true);
                item->setLoading(true);
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
            long long sid = item->sid();
            item->setLoading(true);
            this->cdDir(sid,item->path(),true);
        }
    }else if(sender==ui->actionDock_To_Client){

    }else if(sender==ui->actionOpen){

    }else if(sender==ui->actionDownload){
        QModelIndexList list = model->selectedRows();

    }else if(sender==ui->actionDownload_To){

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
    }
}

void ServerManagePane::onNewFolder(long long id,const QString& path,const QString& name){
    auto req = NetworkManager::getInstance()->request(id);
    if(req!=nullptr){
         auto thread = new ServerRequestThread(req,ServerRequestThread::NewFolder,new QString(path+name));
         connect(thread,&ServerRequestThread::finished,thread,&ServerRequestThread::deleteLater);
         connect(thread,&ServerRequestThread::resultReady, this, &ServerManagePane::onNetworkResponse);
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
         this->addThread(thread);
         thread->start();
    }
}

void ServerManagePane::onThreadFinished(){
    auto sender = static_cast<ServerRequestThread*>(this->sender());
    if(d->list.contains(sender)){
         d->list.removeOne(sender);
    }
}

ServerManagePane* ServerManagePane::open(DockingPaneManager* dockingManager,bool active){
    if(instance==nullptr){
        instance = new ServerManagePane(dockingManager->widget());
        DockingPaneLayoutItemInfo* item = dockingManager->createPane(instance,DockingPaneManager::S_Right,active);
        item->setStretch(300);
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
