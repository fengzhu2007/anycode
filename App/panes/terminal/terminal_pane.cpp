#include "terminal_pane.h"
#include "ui_terminal_pane.h"
#include "docking_pane_layout_item_info.h"
#include "terminal_widget.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event.h"
#include "tab_style.h"
#include <QToolButton>
#include <QMenu>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTabBar>
namespace ady{

//QMap<TerminalPane::Type,QString> TerminalPane::executablelist;
QList<TerminalPane::Excutable> TerminalPane::executablelist;

TerminalPane* TerminalPane::instance = nullptr;

const QString TerminalPane::PANE_ID = "Terminal";
const QString TerminalPane::PANE_GROUP = "Terminal";


class TerminalPanePrivate{
public:
    QToolButton* add;
    QString currentPath;
    QString currentExecutable;
};



TerminalPane::TerminalPane(QWidget *parent,const QString& executable,const QString& workingDir):DockingPane(parent),ui(new Ui::TerminalPane) {
    Subscriber::reg();
    this->regMessageIds({Type::M_OPEN_TERMINAL});
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(tr("Terminal"));
    this->setStyleSheet("QToolBar{border:0px;}"
                        ".ady--TerminalPane>#widget{background-color:#EEEEF2}"
                        "QTabWidget::pane{border:0px solid red}");

    d = new TerminalPanePrivate;
    d->currentExecutable = executable;
    d->currentPath = workingDir;
    //qDebug()<<"TerminalPane"<<d->currentExecutable<<d->currentPath;

    connect(ui->actionAdd,&QAction::triggered,this,&TerminalPane::onActionTriggered);
    connect(ui->actionRemove,&QAction::triggered,this,&TerminalPane::onActionTriggered);
    connect(ui->actionClear,&QAction::triggered,this,&TerminalPane::onActionTriggered);
    connect(ui->actionZoom_In,&QAction::triggered,this,&TerminalPane::onActionTriggered);
    connect(ui->actionZoom_Out,&QAction::triggered,this,&TerminalPane::onActionTriggered);
    connect(ui->tabWidget,&QTabWidget::currentChanged,this,&TerminalPane::onCurrentChanged);

    TerminalPane::toExecutableList();//init


    this->initView();
}


TerminalPane::~TerminalPane(){
    instance = nullptr;
    Subscriber::unReg();
    delete ui;
    delete d;

}

void TerminalPane::initView(){
    ui->tabWidget->tabBar()->setStyle(new TabStyle());
    d->add = new QToolButton(ui->toolBar);
    d->add->setDefaultAction(ui->actionAdd);
    ui->toolBar->insertWidget(ui->actionRemove,d->add);

    if(executablelist.size()==0){
        d->add->setEnabled(false);
    }else if(executablelist.size()>1){
        d->add->setPopupMode(QToolButton::MenuButtonPopup);
        QMenu* menu = new QMenu(d->add);
        for(auto one:executablelist){
            TerminalPane::TerminalType type = one.type;
            menu->addAction(one.icon,one.name,[this,type]{
                this->newTermnal(type,QLatin1String("."));
            });
        }
        d->add->setMenu(menu);
    }
    int count = ui->tabWidget->count();
    if(count==0){
        //init first tab
        if(d->currentPath.isEmpty()){
            d->currentPath = QLatin1String(".");
        }
        //qDebug()<<"executable"<<d->currentExecutable<<d->currentPath;
        if(!d->currentExecutable.isEmpty()){
            this->newTermnal(d->currentExecutable,d->currentPath);
        }else{
            this->newTermnal(TerminalPane::Unkown,d->currentPath);
        }

    }
}

QString TerminalPane::id(){
    return TerminalPane::PANE_ID;
}

QString TerminalPane::group(){

    return TerminalPane::PANE_GROUP;
}

bool TerminalPane::onReceive(Event* e) {
    if(e->id()==Type::M_OPEN_TERMINAL){
        auto workingDir = static_cast<QString*>(e->data());
        this->newTermnal(Unkown,*workingDir);
        return true;
    }
    return false;
}

QJsonObject TerminalPane::toJson() {
    QJsonObject data = {
        {"currentExecutable",d->currentExecutable},
        {"currentPath",d->currentPath},
    };
    return {
        {"id",this->id()},
        {"group",this->group()},
        {"data",data}
    };
}

void TerminalPane::newTermnal(TerminalPane::TerminalType type,const QString& workingDir){
    QString executable;
    for(auto one:executablelist){
        if(type==one.type || type==TerminalPane::Unkown){
            executable = one.executable;
            break;
        }
    }
    if(executable.isEmpty()==false){
        this->newTermnal(executable,workingDir);
    }
}

void TerminalPane::newTermnal(const QString& excutablePath,const QString& workingDir){
    QFileInfo fi(excutablePath);
    const QString name = fi.baseName();
    auto tab = new TerminalWidget(excutablePath,workingDir,ui->tabWidget);
    ui->tabWidget->addTab(tab,name);
    ui->tabWidget->setCurrentWidget(tab);
    this->updateToolBar();

}

TerminalPane* TerminalPane::getInstance(){
    return instance;
}

TerminalPane* TerminalPane::open(DockingPaneManager* dockingManager,bool active,const QJsonObject& data){
    if(instance==nullptr){
        auto executable = data.find("currentExecutable")->toString();
        auto workingDir = data.find("currentPath")->toString();
        instance = new TerminalPane(dockingManager->widget(),executable,workingDir);
        DockingPaneLayoutItemInfo* item = dockingManager->createPane(instance,DockingPaneManager::Bottom,active);
        item->setManualSize(260);
    }
    return instance;
}

TerminalPane* TerminalPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    if(instance==nullptr){
        auto executable = data.find("currentExecutable")->toString();
        auto workingDir = data.find("currentPath")->toString();
        instance = new TerminalPane(dockingManager->widget(),executable,workingDir);
        return instance;
    }
    return nullptr;
}

void TerminalPane::onActionTriggered(){
    auto sender = static_cast<QAction*>(this->sender());
    if(sender==ui->actionAdd){
        if(executablelist.size()>0){
            this->newTermnal(executablelist[0].type,QLatin1String("."));
        }
    }else if(sender==ui->actionRemove){
        if(ui->tabWidget->count()>1){
            auto tab = ui->tabWidget->currentWidget();
            ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
            delete tab;
            this->updateToolBar();
        }
    }else if(sender==ui->actionClear){
        auto tab = static_cast<TerminalWidget*>(ui->tabWidget->currentWidget());
        tab->clearContents();
    }else if(sender==ui->actionZoom_In){
        auto tab = static_cast<TerminalWidget*>(ui->tabWidget->currentWidget());
        tab->zoomIn();
    }else if(sender==ui->actionZoom_Out){
        auto tab = static_cast<TerminalWidget*>(ui->tabWidget->currentWidget());
        tab->zoomOut();
    }
}

void TerminalPane::onCurrentChanged(int index){
    auto widget = static_cast<TerminalWidget*>(ui->tabWidget->widget(index));
    d->currentPath = widget->workingDir();
    d->currentExecutable = widget->executablePath();
}

void TerminalPane::updateToolBar(){
    ui->actionRemove->setEnabled(ui->tabWidget->count()>1);
}

QList<TerminalPane::Excutable> TerminalPane::toExecutableList(){
    if(executablelist.size()==0){
        //find cmd
        //find powershell
        //shell linux

#ifdef Q_OS_WIN
        const QString cmd = QStandardPaths::findExecutable("cmd.exe");
        if(!cmd.isEmpty()){
            executablelist << Excutable{Cmd,QIcon(":/Resource/icons/ImmediateWindow_16x.svg"),tr("Command Prompt"),cmd};
        }
        const QString powershell = QStandardPaths::findExecutable("powershell.exe");
        if(!powershell.isEmpty()){
            executablelist << Excutable{PowerShell,QIcon(":/Resource/icons/PowerShellInteractiveWindow_16x.svg"),tr("PowerShell"),powershell};
        }
#else
#ifdef Q_OS_MAC

        const QString cmd = QStandardPaths::findExecutable("zsh");
        if(!cmd.isEmpty()){
            executablelist << Excutable{Cmd,QIcon(":/Resource/icons/ImmediateWindow_16x.svg"),tr("Bash"),cmd};
        }
        qDebug()<<"bash"<<cmd;
#endif
#endif
    }
    return executablelist;
}

}





