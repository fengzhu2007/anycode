#include "open_project_window.h"
#include "ui_open_project_window.h"
#include "components/message_dialog.h"
#include "project_select_model.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/event.h"
#include "core/event_bus/type.h"
#include "core/theme.h"
#include <QDebug>
namespace ady {
OpenProjectWindow* OpenProjectWindow::instance=nullptr;
OpenProjectWindow::OpenProjectWindow(QWidget* parent)
    :wDialog(parent),
     ui(new Ui::OpenProjectWindow)
{

    auto theme = Theme::getInstance();
    auto backgroundColor = theme->backgroundColor().name(QColor::HexRgb);
    this->setStyleSheet("QPushButton{height:20px}"
                        "QScrollArea{border:0;}"
                        "#footer{background:"+backgroundColor+"}");
    ui->setupUi(this);
    this->resetupUi();
    connect(ui->okButton,&QPushButton::clicked,this,&OpenProjectWindow::onSelected);
    connect(ui->cancelButton,&QPushButton::clicked,this,&OpenProjectWindow::close);

}

OpenProjectWindow::~OpenProjectWindow(){
    instance = nullptr;
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
    if(instance==nullptr){
        instance = new OpenProjectWindow(parent);
    }
    instance->setModal(true);
    instance->show();
    return instance;
}

void OpenProjectWindow::showEvent(QShowEvent* e){
    wDialog::showEvent(e);
    this->initData();
}

void OpenProjectWindow::onSelected()
{
    QList<int> list = ui->listView->selection();
    if(list.size()>0){
        auto model = static_cast<ProjectSelectModel*>(ui->listView->model());
        ProjectRecord one = model->itemAt(list.at(0));
        //emit selectionChanged(one.id);
        Publisher::getInstance()->post(new Event(Type::M_OPEN_PROJECT,&one));
        close();
    }else{
        MessageDialog::error(this,tr("Please select project"));
    }
}

}
