#include "s3_form_general.h"
#include "ui_s3_form_general.h"
#include "components/message_dialog.h"
#include <QDebug>
namespace ady{

S3FormGeneral::S3FormGeneral(QWidget* parent)
    :FormPanel(parent),
    ui(new Ui::S3FormGeneral)
{

    ui->setupUi(this);
    ui->portSpinBox->setValue(21);
}


void S3FormGeneral::initFormData(SiteRecord record)
{
    ui->hostLineEdit->setText(record.host);
    ui->portSpinBox->setValue(record.port);
    ui->usernameLineEdit->setText(record.username);
    ui->passwordLineEdit->setText(record.password);
    ui->pathLineEdit->setText(record.path);
}

bool S3FormGeneral::validateFormData(SiteRecord& record)
{
    //bool ret = true;
    record.host = ui->hostLineEdit->text();
    record.port = ui->portSpinBox->value();
    record.username = ui->usernameLineEdit->text();
    record.password = ui->passwordLineEdit->text();
    record.path = ui->pathLineEdit->text();
    if(record.host.isEmpty()){
        //QMessageBox::information(this,tr("Warning"),tr("Host required"));
        MessageDialog::error(this,tr("Host required"));
        return false;
    }
    //qDebug()<<"port::"<<record.port;
    if(record.port<=0 || record.port>65536){
        MessageDialog::error(this,tr("Invalid port"));
        return false;
    }
    if(record.username.isEmpty()){
        MessageDialog::error(this,tr("Username required"));
        return false;
    }

    if(record.password.isEmpty()){
        MessageDialog::error(this,tr("Password required"));
        return false;
    }

    if(record.path.isEmpty()){
        MessageDialog::error(this,tr("Paath required"));
        return false;
    }
    return true;
}


}
