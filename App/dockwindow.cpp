#include "dockwindow.h"
#include "ui_dockwindow.h"

#include <QMenu>
#include <QMenuBar>
#include <QLabel>
#include <QRandomGenerator>
#include <QApplication>
#include "docking_pane_layout.h"
#include "docking_pane_manager.h"
#include <QDebug>
#include <QLabel>
namespace ady{

    DockWindow::DockWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::DockWindow)
    {
        ui->setupUi(this);

        m_dockingPaneManager = new DockingPaneManager(this);

        this->setCentralWidget(m_dockingPaneManager->widget());
        QString group="label";
        for(int i=0;i<5;i++){
            QLabel* label = new QLabel(QString("LABEL:%1").arg(i),(QWidget* )m_dockingPaneManager->workbench());
            //label->setMinimumSize(QSize(400,400));
            QString id = QString("id:%1").arg(i);
            m_dockingPaneManager->createPane(id,group,QString("title:%1").arg(i),label,DockingPaneManager::Right);
        }
    }

    DockWindow::~DockWindow()
    {
        delete ui;
    }


}
