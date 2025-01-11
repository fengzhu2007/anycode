#include "new_project_zero_widget.h"
#include "ui_new_project_zero_widget.h"
#include "new_project_window.h"
#include "storage/project_storage.h"
#include "storage/site_storage.h"
#include "project_manage_model.h"
#include "components/message_dialog.h"
#include "core/theme.h"
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

    auto theme = Theme::getInstance();
    auto color = theme->color().name(QColor::HexRgb);
    auto backgroundColor = theme->backgroundColor().name(QColor::HexRgb);
    auto textColor = theme->textColor().name(QColor::HexRgb);
    auto secondaryTextColor = theme->secondaryTextColor().name(QColor::HexRgb);
    auto primaryColor = theme->primaryColor().name(QColor::HexRgb);
    auto secondaryColor = theme->secondaryColor().name(QColor::HexRgb);
    auto borderColor = theme->borderColor().name(QColor::HexRgb);


    this->setStyleSheet("#right{background-color:"+backgroundColor+"}"
                        "#left>QLabel{padding-left:8px;}"
                 "ady--ProjectItemWidget QLabel#description{color:"+secondaryTextColor+"}"
                 "ady--ProjectItemWidget QPushButton{padding:0;background-color:transparent;border:1px solid transparent;min-width:16px;min-height:16px;}"
                 "ady--ProjectItemWidget QPushButton:hover{background-color:"+secondaryColor+";border:1px solid "+primaryColor+";}"
                "ady--ITDButton{background-color:"+color+";border:1px solid "+borderColor+";}"
                 "ady--ITDButton[state='hover']{background-color:"+secondaryColor+"}"
                 "ady--ITDButton QLabel#title{font-size:16px;color:"+textColor+"}"
                 "ady--ITDButton QLabel#description{font-size:12px;color:"+secondaryTextColor+"}"
                 "ady--ITDButton QLabel#icon{width:32px;height:32px;}");




    /*
    this->setStyleSheet("ady--ProjectItemWidget QLabel#description{color:#999}"
                        "ady--ProjectItemWidget QPushButton{padding:0;background-color:transparent;border:1px solid transparent;}"
                        "ady--ProjectItemWidget QPushButton:hover{background-color:#e5f1fb;border:1px solid #007acc;}");
    */

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
        connect(model,&ProjectManageModel::editClicked,this,&NewProjectZeroWidget::onProjectEditClicked);
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
            siteStorage.delList(one.id);
            //this->initData();//refresh tree list
            model->itemRemoved(i);
        }
    }
}

}
