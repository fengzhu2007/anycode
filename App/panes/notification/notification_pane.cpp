#include "notification_pane.h"
#include "ui_notification_pane.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event.h"
#include "core/event_bus/event_data.h"
#include <docking_pane_layout_item_info.h>
#include "notification_model.h"
#include "core/theme.h"
namespace ady{

NotificationPane* NotificationPane::instance = nullptr;
QList<NotificationData> NotificationPane::list;

const QString NotificationPane::PANE_ID = "Notification";
const QString NotificationPane::PANE_GROUP = "Notification";



class NotificationPanePrivate{
public:
    NotificationModel* model;

};



NotificationPane::NotificationPane(QWidget *parent)
    : DockingPane(parent),Subscriber()
    , ui(new Ui::NotificationPane)
{

    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(tr("Notification"));
    QString borderColor = Theme::getInstance()->borderColor().name(QColor::HexRgb);
    this->setStyleSheet("QToolBar{border:0px;}"
                        ".ady--NotificationCard{border:1px solid "+borderColor+";"
                                        "border-radius:5px;background:"+Theme::getInstance()->secondaryBackgroundColor().name(QColor::HexRgb)+"}"
                        ".ady--NotificationCard #title{font-weight:bold;}"
                        ".ady--NotificationCard #description{color:#999}"
                        ".ady--ListView{border:0}"
                        ".ady--ListView>QWidget#qt_scrollarea_viewport>.QWidget{border:0;background-color:"+Theme::getInstance()->backgroundColor().name(QColor::HexRgb)+";}");

    Subscriber::reg();
    this->regMessageIds({Type::M_NOTIFICATION});
    d = new NotificationPanePrivate;
    d->model = new NotificationModel(ui->listview);
    ui->listview->setModel(d->model);
    d->model->setDataSource(list);
    connect(ui->actionClear_All,&QAction::triggered,this,&NotificationPane::onActionTriggered);


}

NotificationPane::~NotificationPane()
{
    Subscriber::unReg();
    delete d;
    delete ui;
    instance = nullptr;
}

NotificationPane* NotificationPane::getInstance(){
    return instance;
}

QString NotificationPane::id(){
    return NotificationPane::PANE_ID;
}

QString NotificationPane::group(){

    return NotificationPane::PANE_GROUP;
}


bool NotificationPane::onReceive(Event* e) {
    if(e->id()==Type::M_NOTIFICATION){
        QJsonObject json = e->toJsonOf<NotificationData>().toObject();
        //OutputData
        NotificationData data;
        data.fromJson(json);
        return true;
    }
    return false;
}

void NotificationPane::onActionTriggered(){
    auto sender = this->sender();
    if(sender==ui->actionClear_All){
        d->model->setDataSource({});
        NotificationPane::list.clear();
    }
}

NotificationPane* NotificationPane::open(DockingPaneManager* dockingManager,bool active){
    if(instance==nullptr){
        instance = new NotificationPane(dockingManager->widget());
        DockingPaneLayoutItemInfo* item = dockingManager->createPane(instance,DockingPaneManager::S_Right,active);
        item->setManualSize(260);
    }
    return instance;
}

NotificationPane* NotificationPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    if(instance==nullptr){
        instance = new NotificationPane(dockingManager->widget());
        //NotificationPane::notify({"Notification","标题","详细","2022-01-02"});
        //NotificationPane::notify({"Notification","标题2","详细2","2022-01-02"});
        return instance;
    }
    return nullptr;
}

void NotificationPane::notify(const NotificationData& data){
    NotificationPane::list.append(data);
    if(instance){
        instance->d->model->appendItem(data);
    }
}



}
