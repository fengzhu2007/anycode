#include "new_project_one_widget.h"
#include "ui_new_project_one_widget.h"
#include "new_project_window.h"
#include "storage/project_storage.h"
#include "w_toast.h"
#include <QFileDialog>
#include <QDir>
#include <QStyleOption>
#include <QMessageBox>
#include <QPainter>
#include <QDateTime>
#include <QDebug>
namespace ady{
class NewProjectOneWidgetPrivate{
public:
    ProjectRecord current;
};

NewProjectOneWidget::NewProjectOneWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NewProjectOneWidget)
{
    d = new NewProjectOneWidgetPrivate;

    ui->setupUi(this);

    NewProjectWindow* window = (NewProjectWindow*)this->parentWidget()->parentWidget();
    connect(ui->previous,&QPushButton::clicked,window,&NewProjectWindow::previous);
    connect(ui->save,&QPushButton::clicked,this,&NewProjectOneWidget::onSave);
    connect(ui->selectFile,&QPushButton::clicked,this,&NewProjectOneWidget::onSelectFolder);

    ui->cvs->addItems(QStringList{QString::fromUtf8("Git"),QString::fromUtf8("SVN"),});
    ui->cvs->setCurrentIndex(-1);



    this->initData();
}

NewProjectOneWidget::~NewProjectOneWidget()
{
    delete ui;
    delete d;
}

void NewProjectOneWidget::initData(){
    NewProjectWindow* window = (NewProjectWindow*)this->parentWidget()->parentWidget();
    long long id = window->currentProjectId();
    //qDebug()<<"initData:"<<id;
    if(id>0){
        //edit mode
        ProjectStorage projectStorage;
        d->current = projectStorage.one(id);
        //qDebug()<<"initData:"<<one.name;
        if(d->current.id>0){
            //fill form value
            ui->name->setText(d->current.name);
            ui->path->setText(d->current.path);
            ui->cvs->setCurrentText(d->current.cvs);
            ui->cvs_url->setText(d->current.cvs_url);
        }else{
            //toast
            wToast::showText(tr("Invalid Project"));
            window->previous();
        }
    }else{
        ui->name->clear();
        ui->path->clear();
        ui->cvs->setCurrentIndex(-1);
        ui->cvs_url->clear();
        d->current = {};
    }
}

void NewProjectOneWidget::onNext(){
    NewProjectWindow* window = (NewProjectWindow*)this->parentWidget()->parentWidget();
    //window->setCurrentIndex(2);
    window->next();
}


void NewProjectOneWidget::onSave(){



    QString name = ui->name->text();
    if(name.isEmpty()){
        QMessageBox::information(this,tr("Warning"),tr("Name required"));
        return ;
    }
    QString path = ui->path->text();
    if(path.isEmpty()){
        QMessageBox::information(this,tr("Warning"),tr("Path required"));
        return ;
    }

    d->current.name = name;
    d->current.path = path;
    d->current.cvs = ui->cvs->currentText();
    d->current.cvs_url = ui->cvs_url->text();

    ProjectStorage projectStorage;
    if(d->current.id>0){
        //update
        projectStorage.update(d->current);
        wToast::showText(tr("Project updated successfully"));
    }else{
        //insert
         d->current.datetime = QDateTime::currentDateTime().toSecsSinceEpoch();
        long long ret = projectStorage.insert(d->current);
         if(ret>0){
            d->current.id = ret;
            wToast::showText(tr("Project created successfully"));
            NewProjectWindow* window = (NewProjectWindow*)this->parentWidget()->parentWidget();
            window->previous();
         }else{
            wToast::showText(tr("Project created failed,%1").arg(projectStorage.errorText()));
         }
    }
}

void NewProjectOneWidget::onSelectFolder(){
    QString value = ui->path->text();
    QString defaultPath;
    if(value.isEmpty()){
        defaultPath = QDir::homePath();
    }else{
        QFileInfo fi(value);
        defaultPath = fi.path();
    }
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), defaultPath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dir.isEmpty()==false){
        ui->path->setText(dir);
    }
}

void NewProjectOneWidget::paintEvent(QPaintEvent *e){
    Q_UNUSED(e);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
}
