#include "file_transfer_pane.h"
#include "ui_file_transfer_pane.h"
#include "docking_pane_manager.h"
#include "docking_pane_layout_item_info.h"
#include "core/event_bus/event.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event_data.h"
#include "storage/project_storage.h"
#include "storage/site_storage.h"
#include "file_transfer_model.h"
#include "components/message_dialog.h"
#include "network/network_manager.h"

#include <QMenu>

namespace ady{

FileTransferPane* FileTransferPane::instance = nullptr;

const QString FileTransferPane::PANE_ID = "FileTransfer";
const QString FileTransferPane::PANE_GROUP = "FileTransfer";


class FileTransferPanePrivate{
public:

};

FileTransferPane::FileTransferPane(QWidget *parent) :
    DockingPane(parent),
    ui(new Ui::FileTransferPane)
{
    d = new FileTransferPanePrivate;
    Subscriber::reg();
    this->regMessageIds({Type::M_UPLOAD,Type::M_DOWNLOAD,Type::M_OPEN_PROJECT,Type::M_CLOSE_PROJECT_NOTIFY,Type::M_SITE_ADDED,Type::M_SITE_UPDATED,Type::M_SITE_DELETED});
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(tr("File Transfer"));
    this->setStyleSheet("QToolBar{border:0px;}");

    auto instance = FileTransferModel::getInstance();
    instance->synchronousFromResourceManage();

    ui->treeView->setModel(instance);
    ui->treeView->setTextElideMode(Qt::ElideLeft);
    ui->treeView->setColumnWidth(FileTransferModel::Name,240);
    ui->treeView->setColumnWidth(FileTransferModel::FileSize,100);
    ui->treeView->setColumnWidth(FileTransferModel::Src,360);
    ui->treeView->setColumnWidth(FileTransferModel::Dest,360);
    ui->treeView->setColumnWidth(FileTransferModel::Status,180);

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView,&QTreeView::customContextMenuRequested, this, &FileTransferPane::onContextMenu);

    connect(ui->actionRun,&QAction::triggered,this,&FileTransferPane::onActionTriggered);
    connect(ui->actionStop,&QAction::triggered,this,&FileTransferPane::onActionTriggered);
    connect(ui->actionDelete,&QAction::triggered,this,&FileTransferPane::onActionTriggered);
    connect(ui->actionDelete_Failed,&QAction::triggered,this,&FileTransferPane::onActionTriggered);
    connect(ui->actionDelete_All,&QAction::triggered,this,&FileTransferPane::onActionTriggered);
    connect(ui->actionRetry,&QAction::triggered,this,&FileTransferPane::onActionTriggered);
    connect(ui->actionRetry_All,&QAction::triggered,this,&FileTransferPane::onActionTriggered);

    ui->treeView->expandAll();
    this->initView();
}

FileTransferPane::~FileTransferPane()
{
    Subscriber::unReg();
    delete ui;
    delete d;
    instance = nullptr;
}

void FileTransferPane::initView(){
    ui->actionRun->setEnabled(true);
    ui->actionStop->setEnabled(false);
}

QString FileTransferPane::id(){
    return PANE_ID;
}

QString FileTransferPane::group(){
    return PANE_GROUP;
}

bool FileTransferPane::onReceive(Event* e) {
    const QString id = e->id();
    if(id==Type::M_UPLOAD){

        /*auto data = static_cast<UploadData*>(e->data());
        if(data!=nullptr){
            auto model = static_cast<FileTransferModel*>(ui->treeView->model());
            model->addJob(data);
        }*/
        auto data = e->toJsonOf<UploadData>().toObject();
        if(data.isEmpty()==false){
            auto model = static_cast<FileTransferModel*>(ui->treeView->model());
            model->addUploadJob(data);
            this->scrollToCurrent();
        }
        return true;
    }else if(id==Type::M_DOWNLOAD){
        //auto data = static_cast<DownloadData*>(e->data());
        auto data = e->toJsonOf<DownloadData>().toObject();
        if(data.isEmpty()==false){
            auto model = static_cast<FileTransferModel*>(ui->treeView->model());
            model->addDownloadJob(data);
            this->scrollToCurrent();
        }
        return true;
    }else if(id==Type::M_OPEN_PROJECT){
        //long long id=0;
        QString name;
        auto proj = e->toJsonOf<ProjectRecord>().toObject();
        long long id = proj.find("id")->toInt(0);
        if(id>0){
            ProjectStorage projectStorage;
            auto record = projectStorage.one(id);
            if(record.id>0){
                auto model = static_cast<FileTransferModel*>(ui->treeView->model());
                model->openProject(record.id,record.name,record.path);
                ui->treeView->expandAll();
            }
        }
        return true;
    }else if(id==Type::M_CLOSE_PROJECT_NOTIFY){
        auto data = e->toJsonOf<CloseProjectData>().toObject();
        long long id = data.find("id")->toInt();
        if(id>0){
            auto model = static_cast<FileTransferModel*>(ui->treeView->model());
            auto proj = model->rootItem()->findByProjectId(id);
            QList<int> ids;
            if(proj!=nullptr){
                //find all site
                int total = proj->childrenCount();
                for(int i=0;i<total;i++){
                    auto site = proj->childAt(i);
                    ids<<site->id();

                }
            }
            model->removeProject(id);
            for(auto id:ids){
                model->abortJob(id);
            }
        }
    }else if(id==Type::M_SITE_ADDED){
        auto site = e->toJsonOf<SiteRecord>().toObject();
        auto id = site.find("id")->toInt(0);
        if(id!=0){
            auto one = SiteStorage().one(id);
            if(one.id!=0 && one.status==1){
                auto model = static_cast<FileTransferModel*>(ui->treeView->model());
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
                auto model = static_cast<FileTransferModel*>(ui->treeView->model());
                if(one.status!=1){
                    //remove
                    model->removeSite(one.id);
                }else{
                    auto r = model->find(one.id,false);
                    if(r){
                        model->updateSite(one);
                        //update request client
                         auto req = NetworkManager::getInstance()->request(id);
                        if(req){
                             req->init(one);
                        }
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
            auto model = static_cast<FileTransferModel*>(ui->treeView->model());
            model->removeSite(id);
        }
    }
    return false;
}



void FileTransferPane::onContextMenu(const QPoint& pos){
    QMenu contextMenu(this);
    contextMenu.addAction(ui->actionRun);
    contextMenu.addAction(ui->actionStop);
    contextMenu.addSeparator();

    contextMenu.addAction(ui->actionRetry);
    contextMenu.addAction(ui->actionRetry_All);

    contextMenu.addSeparator();
    contextMenu.addAction(ui->actionDelete);
    contextMenu.addAction(ui->actionDelete_Failed);
    contextMenu.addAction(ui->actionDelete_All);
    contextMenu.exec(QCursor::pos());
}

void FileTransferPane::onActionTriggered(){
    auto sender = static_cast<QAction*>(this->sender());
    if(sender==ui->actionRun){
        ui->actionRun->setEnabled(false);
        ui->actionStop->setEnabled(true);
        auto model = static_cast<FileTransferModel*>(ui->treeView->model());
        model->start(3);
    }else if(sender==ui->actionStop){
        ui->actionRun->setEnabled(true);
        ui->actionStop->setEnabled(false);
        auto model = static_cast<FileTransferModel*>(ui->treeView->model());
        model->stop();
    }else if(sender==ui->actionDelete){
        if(MessageDialog::confirm(this,tr("Are you want delete selected items?"))==QMessageBox::Yes){
            QModelIndexList list = ui->treeView->selectionModel()->selectedRows();
            auto model = static_cast<FileTransferModel*>(ui->treeView->model());
            model->removeItems(list);
        }
    }else if(sender==ui->actionDelete_Failed){
        //delete failed job
        if(MessageDialog::confirm(this,tr("Are you want delete failed items?"))==QMessageBox::Yes){
            auto model = static_cast<FileTransferModel*>(ui->treeView->model());
            model->removeAllItems(FileTransferModelItem::Failed);
        }

    }else if(sender==ui->actionDelete_All){
        //delete all jobs
        if(MessageDialog::confirm(this,tr("Are you want delete all items?"))==QMessageBox::Yes){
            auto model = static_cast<FileTransferModel*>(ui->treeView->model());
            model->removeAllItems();
        }
    }else if(sender==ui->actionRetry){
        //retry select failed
        QModelIndexList list = ui->treeView->selectionModel()->selectedRows();
        auto model = static_cast<FileTransferModel*>(ui->treeView->model());
        model->retryItems(list);
    }else if(sender==ui->actionRetry_All){
        //retry select all failed
        auto model = static_cast<FileTransferModel*>(ui->treeView->model());
        model->retryAllItems();
    }
}

void FileTransferPane::scrollToCurrent(){
    //qDebug()<<"scrollToCurrent";
    auto model = static_cast<FileTransferModel*>(ui->treeView->model());
    //find first has task site index
    int count = model->rowCount();
    for(int i=0;i<count;i++){
        auto parent = model->index(i,FileTransferModel::Name);
        int siteCount = model->rowCount(parent);
        for(int j=0;j<siteCount;j++){
            auto index = model->index(j,FileTransferModel::Name,parent);
            if(model->rowCount(index)>0){
                //auto item = static_cast<FileTransferModelItem*>(index.internalPointer());
                //qDebug()<<"scrollToCurrent"<<index.row()<<item->name();
                ui->treeView->scrollTo(model->index(0,FileTransferModel::Name,index));
                return ;
            }
        }
    }
}

FileTransferPane* FileTransferPane::open(DockingPaneManager* dockingManager,bool active){
    if(instance==nullptr){
        instance = new FileTransferPane(dockingManager->widget());
        DockingPaneLayoutItemInfo* item = dockingManager->createPane(instance,DockingPaneManager::Bottom,active);
        item->setManualSize(220);
    }
    return instance;
}

FileTransferPane* FileTransferPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    if(instance==nullptr){
        instance = new FileTransferPane(dockingManager->widget());
        return instance;
    }
    return nullptr;
}



}
