#include "dbms_pane.h"
#include "panes/db/ui_dbms_pane.h"
#include "ui_dbms_pane.h"

#include <docking_pane_manager.h>
#include <docking_pane_container.h>
#include <docking_pane_layout_item_info.h>
#include <docking_workbench.h>
#include <w_toast.h>

#include "core/event_bus/event.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event_data.h"

#include "storage/db_storage.h"
#include "components/tree_item_delegate.h"

#include "sqlite_driver.h"
#include "sqlite_connect_dialog.h"
#include "dbms_thread.h"
#include "dbms_model.h"

#include <QMenu>
#include <QToolButton>
#include <QJsonObject>

namespace ady{


DBMSPane* DBMSPane::instance = nullptr;

const QString DBMSPane::PANE_ID = "DBMS";
const QString DBMSPane::PANE_GROUP = "DBMS";

class DBMSPanePrivate{
public:
    QToolButton* add;
    QMap<long long,DBDriver*> connectManager;
    DBMSModel* model;


};


DBMSPane::DBMSPane(QWidget *parent)
    : DockingPane(parent)
    , ui(new Ui::DBMSPane)
{

    Subscriber::reg();
    this->regMessageIds({Type::M_NEW_CONNECTION,Type::M_UPDATE_CONNECTION});
    ui->setupUi(this);
    d = new DBMSPanePrivate;
    d->model = new DBMSModel(ui->treeView);
    ui->treeView->setModel(d->model);
    ui->treeView->setItemDelegate(new TreeItemDelegate(ui->treeView));
    this->setWindowTitle(tr("Database"));

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView,&QTreeView::customContextMenuRequested, this, &DBMSPane::onContextMenu);

    connect(ui->actionAdd_Database,&QAction::triggered,this,&DBMSPane::onActionTriggered);
    connect(ui->actionRefresh,&QAction::triggered,this,&DBMSPane::onActionTriggered);
    connect(ui->actionDelete,&QAction::triggered,this,&DBMSPane::onActionTriggered);
    connect(ui->actionOpen_Connection,&QAction::triggered,this,&DBMSPane::onActionTriggered);
    connect(ui->actionClose_Connection,&QAction::triggered,this,&DBMSPane::onActionTriggered);
    connect(ui->actionOpen_Databse,&QAction::triggered,this,&DBMSPane::onActionTriggered);
    connect(ui->actionClose_Database,&QAction::triggered,this,&DBMSPane::onActionTriggered);
    connect(ui->actionEdit_Connection,&QAction::triggered,this,&DBMSPane::onActionTriggered);



    this->registerInitCallback<SQliteDriver>("sqlite","SQLite");//init sqlite
    this->initView();
}

DBMSPane::~DBMSPane()
{
    Subscriber::unReg();
    delete d;
    delete ui;

}




void DBMSPane::initView(){

    d->add = new QToolButton(ui->toolBar);
    d->add->setDefaultAction(ui->actionAdd_Database);
    ui->toolBar->addWidget(d->add);

    if(m_initConnectList.size()==0){
         d->add->setEnabled(false);
    }else if(m_initConnectList.size()>1){
        d->add->setPopupMode(QToolButton::MenuButtonPopup);
        QMenu* menu = new QMenu(d->add);
        for(auto one:m_initConnectList){
            auto driver = one.first.first;
            menu->addAction(one.first.second,[this,driver]{
                this->openConnectDialog(driver,{});
            });
        }
        d->add->setMenu(menu);
    }
    //init all db list
    auto list = DBStorage().all();
    d->model->setDatasource(list);
}

QString DBMSPane::id(){
    return PANE_ID;
}

QString DBMSPane::group(){
    return PANE_GROUP;
}

bool DBMSPane::onReceive(Event* e) {
    const QString id = e->id();
    if(id==Type::M_NEW_CONNECTION){
        auto data = e->toJsonOf<DBRecord>().toObject();
        auto id = data.find("id")->toVariant().toLongLong();
        auto driver = data.find("driver")->toString();
        auto name = data.find("name")->toString();
        d->model->addConnection(id,driver,name);

    }else if(id==Type::M_UPDATE_CONNECTION){
        auto data = e->toJsonOf<DBRecord>().toObject();
        //update name
        auto id = data.find("id")->toVariant().toLongLong();
        auto driver = data.find("driver")->toString();
        auto name = data.find("name")->toString();
        d->model->updateConnection(id,name);
    }
    return false;
}


void DBMSPane::openConnectDialog(const QString& driver,const DBRecord& data){
    auto dialog = new SQLiteConnectDialog({},this);
    dialog->setModal(true);
    dialog->show();
}


void DBMSPane::onContextMenu(const QPoint& pos){
    QMenu contextMenu(this);
    auto model = ui->treeView->selectionModel();
    QModelIndex index = model->currentIndex();
    int total = model->model()->rowCount();
    if(index.isValid()){
        auto item = static_cast<DBMSModelItem*>(index.internalPointer());
        auto type = item->type();
        if(type==DBMSModelItem::Connection){
            bool status = item->status();
            if(status){
                contextMenu.addAction(ui->actionClose_Connection);
            }else{
                contextMenu.addAction(ui->actionOpen_Connection);
            }
            contextMenu.addAction(ui->actionEdit_Connection);
            contextMenu.addSeparator();
            contextMenu.addAction(ui->actionDelete);
        }else if(type==DBMSModelItem::DatabaseItem){
            bool status = item->status();
            if(status){
                contextMenu.addAction(ui->actionClose_Database);
            }else{
                contextMenu.addAction(ui->actionOpen_Databse);
            }
            contextMenu.addSeparator();
            contextMenu.addAction(ui->actionDelete);
        }
        contextMenu.exec(QCursor::pos());
    }

}

void DBMSPane::connectServer(long long id,const QString& path){

}



void DBMSPane::onMessage(const QString& message,int result){
    wToast::showText(message);
}

void DBMSPane::onTreeItemExpanded(const QModelIndex& index){

}

void DBMSPane::onTreeItemDClicked(const QModelIndex& index){

}

void DBMSPane::onActionTriggered(){
    QObject* sender = this->sender();
    if(sender==ui->actionAdd_Database){
        if(m_initConnectList.size()>0){
            this->openConnectDialog(m_initConnectList.begin()->first.first,{});
        }
    }else if(sender==ui->actionOpen_Connection){
        QModelIndex index = ui->treeView->selectionModel()->currentIndex();
        if(index.isValid()){
            auto item = static_cast<DBMSModelItem*>(index.internalPointer());
            DBRecord r = DBStorage().one(item->id());
            if(r.isValid()){
                auto driver = this->openConnect(r);
                if(driver){
                    auto ret = driver->connect();
                    if(ret){
                        //update tree view
                        d->model->updateConnection(r.id,true);
                        //qDebug()<<"dbList:"<<driver->dbList();
                    }


                }
            }
        }
    }else if(sender==ui->actionClose_Connection){
        QModelIndex index = ui->treeView->selectionModel()->currentIndex();
        if(index.isValid()){
            auto item = static_cast<DBMSModelItem*>(index.internalPointer());
            d->model->updateConnection(item->id(),false);
        }
    }else if(sender==ui->actionEdit_Connection){

    }else if(sender==ui->actionOpen_Databse){

    }else if(sender==ui->actionClose_Database){

    }
}


void DBMSPane::onThreadFinished(){

}

void DBMSPane::onOutput(const QString& message,int status){
    QJsonObject json = {
        {"level",status},
        {"source",this->windowTitle()},
        {"content",message}
    };
    Publisher::getInstance()->post(Type::M_OUTPUT,json);
}


DBDriver* DBMSPane::openConnect(const DBRecord& data){
    if(!d->connectManager.contains(data.id)){
        for(auto one:m_initConnectList){
            if(one.first.first==data.driver){
                auto driver = one.second(data);
                d->connectManager.insert(data.id,driver);
                return driver;
            }
        }
        return nullptr;
    }else{
        return d->connectManager.find(data.id).value();
    }
}


DBMSPane* DBMSPane::open(DockingPaneManager* dockingManager,bool active){
    if(instance==nullptr){
        instance = new DBMSPane(dockingManager->widget());
        DockingPaneLayoutItemInfo* item = dockingManager->createPane(instance,DockingPaneManager::S_Right,active);
        item->setManualSize(300);
    }
    return instance;
}

DBMSPane* DBMSPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    if(instance==nullptr){
        instance = new DBMSPane(dockingManager->widget());
        return instance;
    }
    return nullptr;
}

}
