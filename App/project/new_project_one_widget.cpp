#include "new_project_one_widget.h"
#include "ui_new_project_one_widget.h"
#include "new_project_window.h"
#include "storage/ProjectStorage.h"
#include <QStyleOption>
#include <QPainter>
#include <QDebug>
namespace ady{
NewProjectOneWidget::NewProjectOneWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NewProjectOneWidget)
{

    ui->setupUi(this);

    NewProjectWindow* window = (NewProjectWindow*)this->parentWidget()->parentWidget();
    connect(ui->previous,&QPushButton::clicked,window,&NewProjectWindow::previous);
    connect(ui->save,&QPushButton::clicked,this,&NewProjectOneWidget::onSave);

    this->initData();
}

NewProjectOneWidget::~NewProjectOneWidget()
{
    delete ui;
}

void NewProjectOneWidget::initData(){
    NewProjectWindow* window = (NewProjectWindow*)this->parentWidget()->parentWidget();
    long long id = window->currentProjectId();
    //qDebug()<<"initData:"<<id;
    if(id>0){
        //edit mode
        ProjectStorage projectStorage;
        ProjectRecord one = projectStorage.one(id);
        //qDebug()<<"initData:"<<one.name;
        if(one.id>0){
            //fill form value
            ui->name->setText(one.name);
            ui->path->setText(one.path);
            ui->cvs_url->setText(one.cvs_url);
        }else{
            //toast
            window->previous();
        }
    }
}

void NewProjectOneWidget::onNext(){
    NewProjectWindow* window = (NewProjectWindow*)this->parentWidget()->parentWidget();
    //window->setCurrentIndex(2);
    window->next();
}


void NewProjectOneWidget::onSave(){

}

void NewProjectOneWidget::paintEvent(QPaintEvent *e){
    Q_UNUSED(e);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
}
