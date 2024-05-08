#include "ProjectEditWidget.h"
#include "ui_ProjectEditWidget.h"
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
namespace ady{

    ProjectEditWidget::ProjectEditWidget(QWidget* parent)
        :QWidget(parent),
        ui(new Ui::ProjectEditWidgetUI)
    {
        ui->setupUi(this);
        this->id = 0l;
        connect(ui->chooseButton,&QPushButton::clicked,this,&ProjectEditWidget::onChoosePath);
    }

    void ProjectEditWidget::getFormData(ProjectRecord& record)
    {
        record.name = ui->titleLineEdit->text();
        record.path = ui->pathLineEdit->text();
        record.id = this->id;
    }


    void ProjectEditWidget::setFormData(ProjectRecord record)
    {
        ui->titleLineEdit->setText(record.name);
        ui->pathLineEdit->setText(record.path);
        this->id = record.id;
    }

    void ProjectEditWidget::clearFormData()
    {
        this->id = 0l;
        ui->titleLineEdit->clear();
        ui->pathLineEdit->clear();
    }


    void ProjectEditWidget::onChoosePath()
    {
        QString value = ui->pathLineEdit->text();
        QString defaultPath;
        if(value.isEmpty()){
            defaultPath = QDir::homePath();
        }else{
            QFileInfo fi(value);
            defaultPath = fi.path();
        }
        QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), defaultPath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if(dir.isEmpty()==false){
            ui->pathLineEdit->setText(dir);
        }
    }

}
