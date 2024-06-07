#include "idewindow.h"
#include "ui_idewindow.h"
#include "docking_pane_manager.h"
#include "docking_workbench.h"
#include "docking_pane.h"
#include <QLabel>
#include <QDebug>

namespace ady{
IDEWindow::IDEWindow(QWidget *parent) :
    wMainWindow(parent),
    ui(new Ui::IDEWindow)
{
    ui->setupUi(this);
    this->resetupUi();

    connect(ui->actionQuit_Q_tAlt_F4,&QAction::triggered,this,&IDEWindow::onDump);

    ui->toolBar->setFixedHeight(32);

    m_dockingPaneManager = new DockingPaneManager(this);

    this->setCentralWidget(m_dockingPaneManager->widget());
    QString group="label";
    for(int i=0;i<2;i++){
        QLabel* label = new QLabel(QString("LABEL:%1").arg(i),(QWidget* )m_dockingPaneManager->workbench());
        //label->setMinimumSize(QSize(400,400));
        QString id = QString("id:%1").arg(i);
        m_dockingPaneManager->createPane(id,group,QString("title:%1").arg(i),label,i==0?DockingPaneManager::Center:DockingPaneManager::Left);
    }

    QLabel* label = new QLabel(QString("Fixed Window"),(QWidget* )m_dockingPaneManager->workbench());
    m_dockingPaneManager->createFixedPane("11","fix","Fixed Window",label,DockingPaneManager::S_Left);


    connect(m_dockingPaneManager->workbench(),&DockingWorkbench::beforePaneClose,this,&IDEWindow::onBeforePaneClose);
    connect(m_dockingPaneManager->workbench(),&DockingWorkbench::paneClosed,this,&IDEWindow::onPaneClosed);


}

IDEWindow::~IDEWindow()
{
    delete ui;
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
        pane->setCloseEnable(false);
    }
}
}
