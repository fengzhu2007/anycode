#include "new_project_window.h"
#include "ui_new_project_window.h"
#include "new_project_zero_widget.h"
#include "new_project_one_widget.h"
#include "new_project_two_widget.h"
#include <QStyleOption>
#include <QPainter>

namespace ady{
NewProjectWindow* NewProjectWindow::instance=nullptr;

class NewProjectWindowPrivate{
public:
    long long project_id = 0;
    int origin=0;

};

NewProjectWindow::NewProjectWindow(QWidget *parent) :
    wDialog(parent)
{
    this->setStyleSheet("QStackedWidget .QPushButton#next,QStackedWidget .QPushButton#previous,QStackedWidget .QPushButton#save,QStackedWidget .QPushButton#newSite{height:22px}"
                        "QStackedWidget QScrollArea{border:0;}");
    ui = new Ui::NewProjectWindow;
    d = new NewProjectWindowPrivate;
    ui->setupUi(this);
    this->resetupUi();


    ui->stackedWidget->addWidget(new NewProjectZeroWidget(ui->stackedWidget));
    ui->stackedWidget->addWidget(new NewProjectOneWidget(ui->stackedWidget));
    ui->stackedWidget->addWidget(new NewProjectTwoWidget(ui->stackedWidget));
}

NewProjectWindow::~NewProjectWindow()
{
    instance = nullptr;
    delete ui;
    delete d;
}

void NewProjectWindow::setCurrentIndex(int i){
    ui->stackedWidget->setCurrentIndex(i);
    if(i==0){
        NewProjectZeroWidget* widget = (NewProjectZeroWidget*)ui->stackedWidget->currentWidget();
        widget->initData();
    }else if(i==1){
        NewProjectOneWidget* widget = (NewProjectOneWidget*)ui->stackedWidget->currentWidget();
        widget->initData();
    }else if(i==2){
        NewProjectTwoWidget* widget = (NewProjectTwoWidget*)ui->stackedWidget->currentWidget();
        widget->initData();
    }
}

void NewProjectWindow::setCurrentProjectId(long long id){
    d->project_id = id;
}

long long NewProjectWindow::currentProjectId(){
    return d->project_id;
}

void NewProjectWindow::setOrigin(int i){
    d->origin = i;
}

void NewProjectWindow::startup(){
    if(ui->stackedWidget->currentIndex()!=0)
        this->setCurrentIndex(0);
}

void NewProjectWindow::next(){
    int current = ui->stackedWidget->currentIndex();
    current += 1;
    if(current<ui->stackedWidget->count()){
        this->setCurrentIndex(current);
    }
}

void NewProjectWindow::previous(){
    if(d->origin>=0){
        this->setCurrentIndex(d->origin);
    }else{
        int current = ui->stackedWidget->currentIndex();
        current -= 1;
        if(current>=0){
            this->setCurrentIndex(d->origin);
        }
    }
}

NewProjectWindow* NewProjectWindow::open(QWidget* parent){
    if(instance==nullptr){
        instance = new NewProjectWindow(parent);
    }
    instance->startup();
    instance->setModal(true);
    instance->show();
    return instance;
}

}
