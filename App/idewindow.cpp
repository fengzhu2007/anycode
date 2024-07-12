#include "idewindow.h"
#include "ui_idewindow.h"
#include "docking_pane_manager.h"
#include "docking_workbench.h"
#include "docking_pane.h"
#include "docking_pane_client.h"
#include "project/open_project_window.h"
#include "project/new_project_window.h"
#include "panes/resource_manager/resource_manager_pane.h"
#include "panes/code_editor/code_editor_pane.h"
#include "panes/code_editor/code_editor_manager.h"
#include "panes/version_control/version_control_pane.h"

#include "core/event_bus/event.h"
#include "core/event_bus/publisher.h"
#include "core/backend_thread.h"
#include "storage/ProjectStorage.h"
#include "w_toast.h"
#include <QLabel>
#include <QCloseEvent>
#include <QFileDialog>
#include <QDebug>

namespace ady{
IDEWindow::IDEWindow(QWidget *parent) :
    wMainWindow(parent),
    ui(new Ui::IDEWindow)
{
    qRegisterMetaType<QFileInfoList>("QFileInfoList");
    Subscriber::reg();
    this->regMessageIds({ResourceManagerPane::M_OPEN_EDITOR});
    ui->setupUi(this);
    this->resetupUi();

    ui->toolBar->setFixedHeight(32);
    m_dockingPaneManager = new DockingPaneManager(this);
    auto w = m_dockingPaneManager->widget();
    m_dockingPaneManager->workbench()->setContentsMargins(0,0,0,6);
    this->setCentralWidget(w);

    ResourceManagerPane::open(m_dockingPaneManager);

    connect(m_dockingPaneManager->workbench(),&DockingWorkbench::beforePaneClose,this,&IDEWindow::onBeforePaneClose);
    connect(m_dockingPaneManager->workbench(),&DockingWorkbench::paneClosed,this,&IDEWindow::onPaneClosed);
    connect(ui->actionOpen_Project,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionOpen_Folder,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionOpen_File,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionNew_File,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionNew_Project,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionSave_S,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionSave_As_A,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionSave_All_L,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionClose_C,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionClose_Project,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionQuit_Q_tAlt_F4,&QAction::triggered,this,&IDEWindow::close);

    //View
    connect(ui->actionResource_Manage,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionVersion_Control,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionServer,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionTerminal,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionFile_Transfer,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionLog,&QAction::triggered,this,&IDEWindow::onActionTriggered);


    wToastManager::init(this);
    BackendThread::init()->start();
    CodeEditorManager::init(m_dockingPaneManager);
    CodeEditorManager::getInstance()->open("D:/wamp/www/demo.php");

    //ProjectStorage projectStorage
    ProjectRecord record = ProjectStorage().one(27);
    Publisher::getInstance()->post(ResourceManagerPane::M_OPEN_PROJECT,(void*)&record);
}

IDEWindow::~IDEWindow()
{
    wToastManager::destory();
    auto t = BackendThread::getInstance();
    t->stop()->quit();
    t->wait();
    CodeEditorManager::destory();
    Subscriber::unReg();
    delete ui;
}

bool IDEWindow::onReceive(Event* e){
    if(e->id()==ResourceManagerPane::M_OPEN_EDITOR){
        auto one = static_cast<QString*>(e->data());
        CodeEditorManager::getInstance()->open(m_dockingPaneManager,*one);
        e->ignore();
        return true;
    }
    return false;
}

void IDEWindow::onDump(){
    m_dockingPaneManager->workbench()->dump("dump");
}

void IDEWindow::onPaneClosed(QString&id,QString&group,bool isClient){
    qDebug()<<"onPaneClosed:"<<id<<";"<<group<<";"<<isClient;
}
void IDEWindow::onBeforePaneClose(DockingPane* pane,bool isClient){
    qDebug()<<"onBeforePaneClose:"<<pane<<";"<<isClient;
    if(isClient){
        //pane->setCloseEnable(false);
    }
}

void IDEWindow::onActionTriggered(){
    QObject* sender = this->sender();
    if(sender==ui->actionOpen_Project){
        OpenProjectWindow::open(this);
    }else if(sender==ui->actionOpen_Folder){
        QString dir = QFileDialog::getExistingDirectory(this, tr("Open Folder"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        this->onOpenFolder(dir);
    }else if(sender==ui->actionOpen_File){
        QString path = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("All Files (*.*)"));
        this->onOpenFile(path);
    }else if(sender==ui->actionNew_Project){
        NewProjectWindow::open(this);
    }else if(sender==ui->actionNew_File){
        CodeEditorManager::getInstance()->open(m_dockingPaneManager,"");
    }else if(sender==ui->actionSave_S || sender==ui->actionSave_As_A){
        //save
        //save as
        DockingPaneClient* client = m_dockingPaneManager->workbench()->client();
        if(client!=nullptr){
            auto pane = client->currentPane();
            if(pane!=nullptr){
                pane->save(sender==ui->actionSave_As_A);
            }
        }
    }else if(sender==ui->actionSave_All_L){
        int count = m_dockingPaneManager->workbench()->clientCount();
        for(int i=0;i<count;i++){
            auto client = m_dockingPaneManager->workbench()->client(i);
            int paneCount = client->paneCount();
            for(int j=0;j<paneCount;j++){
                auto pane = client->pane(j);
                pane->save();
            }
        }
    }else if(sender==ui->actionClose_C){
        DockingPaneClient* client = m_dockingPaneManager->workbench()->client();
        if(client!=nullptr){
            client->closeCurrent();
        }
    }else if(sender==ui->actionClose_Project){

    }else if(sender==ui->actionResource_Manage){
        auto pane = ResourceManagerPane::open(m_dockingPaneManager,true);
        pane->activeToCurrent();
    }else if(sender==ui->actionVersion_Control){
        auto pane = VersionControlPane::open(m_dockingPaneManager,true);
        pane->activeToCurrent();
    }else if(sender==ui->actionServer){

    }else if(sender==ui->actionTerminal){

    }else if(sender==ui->actionFile_Transfer){

    }else if(sender==ui->actionLog){

    }








}

void IDEWindow::onOpenFile(const QString& path){
    CodeEditorManager::getInstance()->open(m_dockingPaneManager,path);
}
void IDEWindow::onOpenFolder(const QString& path){
    ProjectRecord record;
    record.path = path;
    QFileInfo fi(path);
    record.name = fi.fileName();
    Publisher::getInstance()->post(ResourceManagerPane::M_OPEN_PROJECT,(void*)&record);
}

/*void IDEWindow::closeEvent(QCloseEvent *event){

    event->accept();
}*/


}
