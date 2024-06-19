#include "new_project_zero_widget.h"
#include "ui_new_project_zero_widget.h"
#include "new_project_window.h"
#include "storage/ProjectStorage.h"
#include "project_manage_model.h"
#include <QDebug>

namespace ady {
NewProjectZeroWidget::NewProjectZeroWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NewProjectZeroWidget)
{
    ui->setupUi(this);
    ui->newProject->setIcon(QString::fromUtf8(":/Resource/icons/NewFileCollection_16x.svg"));
    ui->newProject->setText(tr("New Project"));
    ui->newProject->setDescription(tr("Create a new project"));
    connect(ui->newProject,&ITDButton::clicked,this,&NewProjectZeroWidget::onNewProject);
    connect(ui->listView,&ListView::itemClicked,this,&NewProjectZeroWidget::onProjectItemClicked);


    this->initData();
}

NewProjectZeroWidget::~NewProjectZeroWidget()
{
    delete ui;
}

void NewProjectZeroWidget::initData(){
    ProjectStorage projectStorage;
    QList<ProjectRecord>projects = projectStorage.all();
    if(ui->listView->model()==nullptr){
        ProjectManageModel* model = new ProjectManageModel(ui->listView);
        model->setDataSource(projects);
        ui->listView->setModel(model);
    }
}

void NewProjectZeroWidget::onNewProject(){
    NewProjectWindow* window = (NewProjectWindow*)this->parentWidget()->parentWidget();
    window->setCurrentProjectId(0);
    //window->setCurrentIndex(1);
    window->next();
}

void NewProjectZeroWidget::onProjectItemClicked(int i){
    ProjectManageModel* model = ( ProjectManageModel*)ui->listView->model();
    ProjectRecord one = model->itemAt(i);

    NewProjectWindow* window = (NewProjectWindow*)this->parentWidget()->parentWidget();
    window->setCurrentProjectId(one.id);
    window->setOrigin(0);
    window->setCurrentIndex(2);
}

void NewProjectZeroWidget::onProjectEditClicked(int i){
    ProjectManageModel* model = ( ProjectManageModel*)ui->listView->model();
    ProjectRecord one = model->itemAt(i);
    NewProjectWindow* window = (NewProjectWindow*)this->parentWidget()->parentWidget();
    window->setCurrentProjectId(one.id);
    window->setOrigin(0);
    window->next();
}

}
