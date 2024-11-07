#include "terminal_pane.h"
#include "ui_terminal_pane.h"
#include "docking_pane_layout_item_info.h"
#include "terminalview.h"
#include <QToolButton>
namespace ady{


TerminalPane* TerminalPane::instance = nullptr;

const QString TerminalPane::PANE_ID = "Terminal";
const QString TerminalPane::PANE_GROUP = "Terminal";


class TerminalPanePrivate{
public:
    QToolButton* add;

};



TerminalPane::TerminalPane(QWidget *parent):DockingPane(parent),ui(new Ui::TerminalPane) {



    Subscriber::reg();
    this->regMessageIds({});
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(tr("Terminal"));
    this->setStyleSheet("QToolBar{border:0px;}"
                        ".ady--TerminalPane>#widget{background-color:#EEEEF2}");

    d = new TerminalPanePrivate;
    d->add = new QToolButton(ui->toolBar);
    d->add->setDefaultAction(ui->actionAdd);
    d->add->setPopupMode(QToolButton::MenuButtonPopup);
    ui->toolBar->insertWidget(ui->actionRemove,d->add);

    this->initView();
}


TerminalPane::~TerminalPane(){
    Subscriber::unReg();
    delete ui;
    delete d;
}

void TerminalPane::initView(){
    int count = ui->tabWidget->count();
    if(count==0){
        //init first tab
        auto tab = new TerminalSolution::TerminalView(ui->tabWidget);
        ui->tabWidget->addTab(tab,tr("One"));
        /*QTimer::singleShot(100,[this,tab]{
            tab->writeToTerminal((tr("Connecting...") + "\r\n").toUtf8(), true);
        });*/

    }

}

QString TerminalPane::id(){
    return TerminalPane::PANE_ID;
}

QString TerminalPane::group(){

    return TerminalPane::PANE_GROUP;
}

bool TerminalPane::onReceive(Event* e) {

    return false;
}


TerminalPane* TerminalPane::open(DockingPaneManager* dockingManager,bool active){
    if(instance==nullptr){
        instance = new TerminalPane(dockingManager->widget());
        DockingPaneLayoutItemInfo* item = dockingManager->createPane(instance,DockingPaneManager::Bottom,active);
        item->setManualSize(200);
    }
    return instance;
}

TerminalPane* TerminalPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    if(instance==nullptr){
        instance = new TerminalPane(dockingManager->widget());
        return instance;
    }
    return nullptr;
}



}





