#include "output_pane.h"
#include "ui_output_pane.h"
#include "docking_pane_layout_item_info.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event.h"
#include "core/event_bus/event_data.h"
#include "core/theme.h"
#include "components/list_item_delegate.h"
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
    this->regMessageIds({Type::M_OUTPUT});
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(tr("Output"));
    this->setStyleSheet("QToolBar{border:0px;}"
                        "QTextBrowser{border:0;background-color:"+Theme::getInstance()->backgroundColor().name(QColor::HexRgb)+";}");

    d = new OutputPanePrivate;


    connect(ui->actionClear,&QAction::triggered,this,&OutputPane::onActionTriggered);
    connect(ui->actionAuto_Wrapping,&QAction::triggered,this,&OutputPane::onActionTriggered);
    connect(ui->source,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&OutputPane::onIndexChanged);

    ui->source->setItemDelegate(new ListItemDelegate(ui->source));


    this->initView();
}


OutputPane::~OutputPane(){
    Subscriber::unReg();
    instance = nullptr;
    delete ui;
    delete d;
}

void OutputPane::initView(){
    ui->actionAuto_Wrapping->setChecked(ui->textBrowser->lineWrapMode()==QTextEdit::NoWrap?false:true);
    ui->source->addItem(tr("All"));

}

QString OutputPane::id(){
    return OutputPane::PANE_ID;
}

QString OutputPane::group(){

    return OutputPane::PANE_GROUP;
}

static QString levelStr(int level){
    if(level==Type::Text){
        return {};
    }else if(level==Type::Ok){
        return QLatin1String("<font color=green>[OK]</font> ");
    }else if(level==Type::Warning){
        return QLatin1String("<font color=orange>[Waring]</font> ");
    }else if(level==Type::Error){
        return QLatin1String("<font color=red>[Error]</font> ");
    }else{
        return {};
    }
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
        if(ui->source->currentIndex()==0 || ui->source->currentText()==data.source){
            ui->textBrowser->append(levelStr(data.level) + data.content);
            ui->textBrowser->moveCursor(QTextCursor::End);
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
        d->contentlist.clear();
    }else if(sender==ui->actionAuto_Wrapping){
        ui->textBrowser->setLineWrapMode(ui->actionAuto_Wrapping->isChecked()?QTextEdit::WidgetWidth:QTextEdit::NoWrap);
    }
}

void OutputPane::onIndexChanged(int index){
    const QString text = ui->source->currentText();
    ui->textBrowser->clear();
    for(auto one:d->contentlist){
        if(index==0 || one.source==text)
            ui->textBrowser->append(levelStr(one.level) + one.content);
    }
}

}
