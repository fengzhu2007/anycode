#include "open_project_window.h"
#include "ui_open_project_window.h"
#include "components/message_dialog.h"
#include "project_select_model.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/event.h"
#include "panes/resource_manager/resource_manager_pane.h"
#include <QDebug>
namespace ady {
OpenProjectWindow::OpenProjectWindow(QWidget* parent)
    :wDialog(parent),
     ui(new Ui::OpenProjectWindow)
{
    this->setStyleSheet("QPushButton{height:22px}"
                        "QScrollArea{border:0;}");
    ui->setupUi(this);
    this->resetupUi();
    connect(ui->okButton,&QPushButton::clicked,this,&OpenProjectWindow::onSelected);
    connect(ui->cancelButton,&QPushButton::clicked,this,&OpenProjectWindow::close);
    this->initData();
}

void OpenProjectWindow::initData()
{
    ProjectStorage projectStorage;
    QList<ProjectRecord>lists = projectStorage.all();
    auto model = new ProjectSelectModel(ui->listView);
    model->setDataSource(lists);
    ui->listView->setModel(model);
}

OpenProjectWindow* OpenProjectWindow::open(QWidget* parent){
    OpenProjectWindow* window = new OpenProjectWindow(parent);
    window->setModal(true);
    window->show();
    return window;
}

void OpenProjectWindow::onSelected()
{
    QList<int> list = ui->listView->selection();
    if(list.size()>0){
        auto model = static_cast<ProjectSelectModel*>(ui->listView->model());
        ProjectRecord one = model->itemAt(list.at(0));
        //emit selectionChanged(one.id);
        Publisher::getInstance()->post(new Event(ResourceManagerPane::M_OPEN_PROJECT,&one));
        close();
    }else{
        MessageDialog::error(this,tr("Please select project"));
    }
}

}
