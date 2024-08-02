#include "new_project_zero_widget.h"
#include "ui_new_project_zero_widget.h"
#include "new_project_window.h"
#include "storage/project_storage.h"
#include "storage/site_storage.h"
#include "project_manage_model.h"
#include "components/message_dialog.h"
#include <QMenu>
#include <QCursor>
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

    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);


    connect(ui->newProject,&ITDButton::clicked,this,&NewProjectZeroWidget::onNewProject);
    connect(ui->listView,&ListView::itemClicked,this,&NewProjectZeroWidget::onProjectItemClicked);
    connect(ui->listView, &QWidget::customContextMenuRequested, this, &NewProjectZeroWidget::showProjectContextMenu);

    this->initData();
}

NewProjectZeroWidget::~NewProjectZeroWidget()
{
    delete ui;
}

void NewProjectZeroWidget::initData(){
    ProjectStorage projectStorage;
    QList<ProjectRecord>projects = projectStorage.all();
    auto model = static_cast<ProjectManageModel*>(ui->listView->model());
    if(model==nullptr){
        model = new ProjectManageModel(ui->listView);
        model->setDataSource(projects);
        ui->listView->setModel(model);
    }else{
        model->setDataSource(projects);
    }
}

void NewProjectZeroWidget::onNewProject(){
    NewProjectWindow* window = (NewProjectWindow*)this->parentWidget()->parentWidget();
    window->setCurrentProjectId(0);
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

void NewProjectZeroWidget::showProjectContextMenu(const QPoint &pos){
    int row = ui->listView->findItem(pos);
    if(row>=0){
        QMenu contextMenu(this);
        contextMenu.addAction(tr("Edit"),[this,row](){
            this->onProjectEditClicked(row);
        });
        contextMenu.addAction(tr("Delete"),[this,row](){
            this->onDeleteProject(row);
        });
        contextMenu.exec(QCursor::pos());

    }
}

void NewProjectZeroWidget::onDeleteProject(int i){
    auto model = static_cast< ProjectManageModel*>(ui->listView->model());
    if(model!=nullptr){
        ProjectRecord one = model->itemAt(i);
        if(MessageDialog::confirm(this,tr("Are you want to delete selected project[%1] and sites?").arg(one.name),QMessageBox::Ok|QMessageBox::Cancel)==QMessageBox::Ok){
            ProjectStorage db;
            db.del(one.id);
            SiteStorage siteStorage;
            siteStorage.del_list(one.id);
            //this->initData();//refresh tree list
            model->itemRemoved(i);
        }
    }
}

}
