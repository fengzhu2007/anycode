#include "frames_viewer_pane.h"
#include "ui_frames_viewer_pane.h"
#include "animation_frames_player.h"
#include "frames_model.h"
#include "frames_merge_dialog.h"
#include <docking_pane_layout_item_info.h>
#include <w_toast.h>
#include <QVBoxLayout>
#include <QDir>
#include <QTimer>
#include <QMenu>
namespace ady{

const QString FramesViewerPane::PANE_ID = "FramesViewer_%1";
const QString FramesViewerPane::PANE_GROUP = "FramesViewer";
int FramesViewerPane::SN = 0;


class FramesViewerPanePrivate{
public:
    int id;
    AnimationFramesPlayer* player;
    FramesModel* model;
    QTimer timer;
    int current = 0;
};

FramesViewerPane::FramesViewerPane(QWidget *parent)
    : DockingPane(parent)
    , ui(new Ui::FramesViewerPane)
{
    d = new FramesViewerPanePrivate;
    d->id = FramesViewerPane::SN++;
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    d->player = new AnimationFramesPlayer(widget);
    auto layout = static_cast<QVBoxLayout*>(widget->layout());
    layout->insertWidget(0,d->player,1);
    this->setWindowTitle(tr("Frames Viewer"));

    d->model = new FramesModel(ui->listView);
    ui->listView->setModel(d->model);
    ui->listView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(ui->load,&QPushButton::clicked,this,&FramesViewerPane::onLoad);
    connect(ui->listView,&QListView::doubleClicked,this,&FramesViewerPane::onDoubleClicked);

    connect(ui->listView,&QListView::customContextMenuRequested,this,&FramesViewerPane::onListContextMenu);
    connect(&d->timer,&QTimer::timeout,this,&FramesViewerPane::onFrameChange);
    connect(ui->actionPlay,&QAction::triggered,this,&FramesViewerPane::onActionTriggered);
    connect(ui->actionEnable,&QAction::triggered,this,&FramesViewerPane::onActionTriggered);
    connect(ui->actionDisable,&QAction::triggered,this,&FramesViewerPane::onActionTriggered);
    connect(ui->actionMerge,&QAction::triggered,this,&FramesViewerPane::onActionTriggered);
}

FramesViewerPane::~FramesViewerPane()
{
    delete ui;
    delete d;
}

void FramesViewerPane::initView(){

}

QString FramesViewerPane::id() {

    return PANE_ID.arg(d->id);
}
QString FramesViewerPane::group(){
    return PANE_GROUP;
}
FramesViewerPane* FramesViewerPane::open(DockingPaneManager* dockingManager,bool active){
    auto pane = new FramesViewerPane(dockingManager->widget());
    DockingPaneLayoutItemInfo* item = dockingManager->createPane(pane,DockingPaneManager::Center,active);
    return pane;
}

FramesViewerPane* FramesViewerPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    auto pane = FramesViewerPane::open(dockingManager,true);
    return pane;
}

void FramesViewerPane::onLoad(){
    auto folder = ui->folder->text();
    QDir dir(folder);
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg";
    dir.setNameFilters(filters);


    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name|QDir::DirsFirst|QDir::IgnoreCase);
    QStringList array;

    for(auto file:list){
        array.append(file.absoluteFilePath());
    }
    d->model->setDatasource(array);
    if(array.size()==0){
        wToast::showText(tr("No picture found."));
    }
}

void FramesViewerPane::onDoubleClicked(const QModelIndex& index){
    d->player->load(d->model->image(index.row()));
}
void FramesViewerPane::onActionTriggered(){
    auto sender = this->sender();
    if(sender==ui->actionPlay){
        if(d->model->rowCount()<=0){
            return ;
        }
        d->current = 0;
        d->timer.setInterval(200);
        d->timer.setSingleShot(false);
        d->timer.start();
    }else if(sender==ui->actionEnable){
        auto indexlist = ui->listView->selectionModel()->selectedIndexes();
        for(auto index:indexlist){
            d->model->setFrameStatus(index.row(),true);
        }
    }else if(sender==ui->actionDisable){
        auto indexlist = ui->listView->selectionModel()->selectedIndexes();
        for(auto index:indexlist){
            d->model->setFrameStatus(index.row(),false);
        }
    }else if(sender==ui->actionMerge){
        auto dialog = FramesMergeDialog::open(this);
        dialog->setModel(d->model);
        dialog->saveTo(ui->folder->text());
        dialog->show();

    }
}
void FramesViewerPane::onFrameChange(){
    auto total = d->model->rowCount();
    if(total<=0){
        return ;
    }

    for(int i=0;i<total;i++){
         d->current = d->current % total;
        auto frame = d->model->at(d->current);
        d->current += 1;
        if(frame.status){
            d->player->load(frame.image);
            return ;
        }
    }
    d->player->load(d->model->image(0));
}

void FramesViewerPane::onListContextMenu(const QPoint &pos){
    QMenu contextMenu(this);

    auto view = static_cast<QAbstractItemView*>(this->sender());
    auto model = view->selectionModel();
    QModelIndex index = model->currentIndex();
    //int total = model->model()->rowCount();
    if(index.isValid()){
        contextMenu.addAction(ui->actionEnable);
        contextMenu.addAction(ui->actionDisable);
        contextMenu.exec(QCursor::pos());
    }
}

}
