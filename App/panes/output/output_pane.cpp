#include "output_pane.h"
#include "ui_output_pane.h"
#include "docking_pane_layout_item_info.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event.h"
#include "core/event_bus/event_data.h"
#include <QAction>

namespace ady{


OutputPane* OutputPane::instance = nullptr;

const QString OutputPane::PANE_ID = "Output";
const QString OutputPane::PANE_GROUP = "Output";


class OutputPanePrivate{
public:
    QStringList sourcelist;
    QList<OutputData> contentlist;

};



OutputPane::OutputPane(QWidget *parent):DockingPane(parent),ui(new Ui::OutputPane) {
    Subscriber::reg();
    this->regMessageIds({});
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(tr("Output"));
    this->setStyleSheet("QToolBar{border:0px;}"
                        ".ady--OutputPane>#widget{background-color:#EEEEF2}"
                        ".ady--OutputPane QTextBrowser{border:0;background-color:#E6E7E8;}");

    d = new OutputPanePrivate;


    connect(ui->actionClear,&QAction::triggered,this,&OutputPane::onActionTriggered);
    connect(ui->actionAuto_Wrapping,&QAction::triggered,this,&OutputPane::onActionTriggered);


    this->initView();
}


OutputPane::~OutputPane(){
    Subscriber::unReg();
    delete ui;
    delete d;
}

void OutputPane::initView(){
    //ui->textBrowser->setText("111111111111111111111111111111111111111111111111111111122222222");

}

QString OutputPane::id(){
    return OutputPane::PANE_ID;
}

QString OutputPane::group(){

    return OutputPane::PANE_GROUP;
}

bool OutputPane::onReceive(Event* e) {
    if(e->id()==Type::M_OUTPUT){
        QJsonObject json = e->toJsonOf<OutputData>().toObject();
        //OutputData
        OutputData data;
        data.fromJson(json);
        d->contentlist<<data;
        if(data.source.isEmpty()==false && !d->sourcelist.contains(data.source)){
            d->sourcelist<<data.source;
            //update combobox
            ui->source->addItem(data.source);
        }
        return true;
    }
    return false;
}

OutputPane* OutputPane::open(DockingPaneManager* dockingManager,bool active){
    if(instance==nullptr){
        instance = new OutputPane(dockingManager->widget());
        DockingPaneLayoutItemInfo* item = dockingManager->createPane(instance,DockingPaneManager::Bottom,active);
        item->setManualSize(260);
    }
    return instance;
}

OutputPane* OutputPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    if(instance==nullptr){
        instance = new OutputPane(dockingManager->widget());
        return instance;
    }
    return nullptr;
}

void OutputPane::onActionTriggered(){
    auto sender = static_cast<QAction*>(this->sender());
    if(sender==ui->actionClear){
        ui->textBrowser->clear();
    }else if(sender==ui->actionAuto_Wrapping){
        ui->textBrowser->setLineWrapMode(ui->actionAuto_Wrapping->isChecked()?QTextEdit::WidgetWidth:QTextEdit::NoWrap);
    }
}

}
