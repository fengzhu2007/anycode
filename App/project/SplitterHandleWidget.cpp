#include "SplitterHandleWidget.h"
#include "ui_splitterhandlewidgetui.h"
#include <QDebug>
namespace ady {
   SplitterHandleWidget::SplitterHandleWidget(QWidget* parent)
       :QWidget(parent),
        ui(new Ui::SplitterHandleWidgetUI){
        ui->setupUi(this);


        ui->actionNew_Project->setData(A_NEW_PROJECT);
        ui->actionDelete_Project->setData(A_DEL_PROJECT);
        ui->actionNew_Site->setData(A_NEW_SITE);
        ui->actionDelete_Site->setData(A_DEL_SITE);
        ui->actionMove_Up->setData(A_MOVE_UP);
        ui->actionMove_Down->setData(A_MOVE_DOWN);

        connect(ui->actionNew_Project,&QAction::triggered,this,&SplitterHandleWidget::onTriggered);
        connect(ui->actionDelete_Project,&QAction::triggered,this,&SplitterHandleWidget::onTriggered);
        connect(ui->actionNew_Site,&QAction::triggered,this,&SplitterHandleWidget::onTriggered);
        connect(ui->actionDelete_Site,&QAction::triggered,this,&SplitterHandleWidget::onTriggered);
        connect(ui->actionMove_Up,&QAction::triggered,this,&SplitterHandleWidget::onTriggered);
        connect(ui->actionMove_Down,&QAction::triggered,this,&SplitterHandleWidget::onTriggered);

   }

   SplitterHandleWidget::~SplitterHandleWidget()
   {

   }

   void SplitterHandleWidget::onTriggered(bool checked)
   {
        emit actionTriggered(static_cast<QAction*>(sender()),checked);
   }


   void SplitterHandleWidget::setActionGroupStatus(int role)
   {
      switch (role) {
          case 0://project
                ui->actionDelete_Project->setEnabled(true);
                ui->actionNew_Site->setEnabled(true);
                ui->actionDelete_Site->setEnabled(false);
                ui->actionMove_Up->setEnabled(false);
                ui->actionMove_Down->setEnabled(false);


          break;
          case 1://group
              ui->actionDelete_Project->setEnabled(false);
              ui->actionNew_Site->setEnabled(true);
              ui->actionDelete_Site->setEnabled(false);
              ui->actionMove_Up->setEnabled(false);
              ui->actionMove_Down->setEnabled(false);
          break;
          case 2://site
              ui->actionDelete_Project->setEnabled(false);
              ui->actionNew_Site->setEnabled(false);
              ui->actionDelete_Site->setEnabled(true);
              ui->actionMove_Up->setEnabled(true);
              ui->actionMove_Down->setEnabled(true);
          break;

      }
   }

}
