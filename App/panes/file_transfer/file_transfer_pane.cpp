#include "file_transfer_pane.h"
#include "ui_file_transfer_pane.h"
#include "docking_pane_manager.h"
#include "docking_pane_layout_item_info.h"
#include "core/event_bus/event.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event_data.h"
#include "storage/project_storage.h"
#include "file_transfer_model.h"

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
    this->regMessageIds({Type::M_UPLOAD,Type::M_DOWNLOAD,Type::M_OPEN_PROJECT});
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(tr("File Transfer"));
    this->setStyleSheet("QToolBar{border:0px;}"
                        "QTreeView{border:0;background-color:#f5f5f5}"
                        ".ady--FileTransferPane>#widget{background-color:#EEEEF2}");



    ui->treeView->setModel(new FileTransferModel(ui->treeView));
    ui->treeView->setTextElideMode(Qt::ElideLeft);
    ui->treeView->setColumnWidth(FileTransferModel::Name,240);
    ui->treeView->setColumnWidth(FileTransferModel::FileSize,100);
    ui->treeView->setColumnWidth(FileTransferModel::Src,360);
    ui->treeView->setColumnWidth(FileTransferModel::Dest,360);
    ui->treeView->setColumnWidth(FileTransferModel::Status,180);

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView,&QTreeView::customContextMenuRequested, this, &FileTransferPane::onContextMenu);

    this->initView();
}

FileTransferPane::~FileTransferPane()
{
    delete ui;
    delete d;
    instance = nullptr;
}

void FileTransferPane::initView(){

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

    }else if(id==Type::M_DOWNLOAD){

    }else if(id==Type::M_OPEN_PROJECT){
        auto one = static_cast<ProjectRecord*>(e->data());

        if(one->id>0){
            auto model = static_cast<FileTransferModel*>(ui->treeView->model());
            model->openProject(one->id,one->name);
        }
        return true;
    }
    return false;
}

void FileTransferPane::onContextMenu(const QPoint& pos){
    QMenu contextMenu(this);
    contextMenu.addAction(ui->actionRun);
    contextMenu.addAction(ui->actionStop);

    //contextMenu.addSection(tr("Delete"));
    contextMenu.addSeparator();
    contextMenu.addAction(ui->actionDelete);
    contextMenu.exec(QCursor::pos());
}

FileTransferPane* FileTransferPane::open(DockingPaneManager* dockingManager,bool active){
    if(instance==nullptr){
        instance = new FileTransferPane(dockingManager->widget());
        DockingPaneLayoutItemInfo* item = dockingManager->createPane(instance,DockingPaneManager::Bottom,active);
        item->setStretch(220);
    }
    return instance;
}


}
