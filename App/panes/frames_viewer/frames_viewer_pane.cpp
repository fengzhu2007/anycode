#include "frames_viewer_pane.h"
#include "ui_frames_viewer_pane.h"
#include "animation_frames_player.h"
#include "frames_model.h"
#include <docking_pane_layout_item_info.h>
#include <w_toast.h>
#include <QVBoxLayout>
#include <QDir>
#include <QTimer>
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
    connect(ui->load,&QPushButton::clicked,this,&FramesViewerPane::onLoad);
    connect(ui->listView,&QListView::doubleClicked,this,&FramesViewerPane::onDoubleClicked);
    connect(ui->actionPlay,&QAction::triggered,this,&FramesViewerPane::onActionTriggered);
    connect(&d->timer,&QTimer::timeout,this,&FramesViewerPane::onFrameChange);
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
    item->setManualSize(260);
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
    // QDir output(d->destination);
    // if(!output.exists()){
    //     output.mkdir();
    // }

    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name|QDir::DirsFirst|QDir::IgnoreCase);
    QStringList array;


    //void setList(QList<FileItem>& data);
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
        d->timer.setInterval(50);
        d->timer.setSingleShot(false);
        d->timer.start();
    }
}
void FramesViewerPane::onFrameChange(){
    auto total = d->model->rowCount();

    d->current = d->current % total;
    d->player->load(d->model->image(d->current));
    d->current += 1;
}
}
