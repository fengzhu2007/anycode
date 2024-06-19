#include "COSFormGeneral.h"
#include "ui_COSFormGeneral.h"
#include "components/MessageDialog.h"
#include <QDebug>
namespace ady{

COSFormGeneral::COSFormGeneral(QWidget* parent)
    :FormPanel(parent),
    ui(new Ui::COSFormGeneralUI)
{

    ui->setupUi(this);
    ui->portSpinBox->setValue(80);
}


void COSFormGeneral::initFormData(SiteRecord record)
{
    ui->hostLineEdit->setText(record.host);
    ui->portSpinBox->setValue(record.port);
    ui->usernameLineEdit->setText(record.username);
    ui->passwordLineEdit->setText(record.password);
    ui->pathLineEdit->setText(record.path);
    //ui->statusCheckBox->setCheckState(record.status==1?Qt::Checked:Qt::Unchecked);
}

bool COSFormGeneral::validateFormData(SiteRecord& record)
{
    //bool ret = true;
    record.host = ui->hostLineEdit->text();
    record.port = ui->portSpinBox->value();
    record.username = ui->usernameLineEdit->text();
    record.password = ui->passwordLineEdit->text();
    record.path = ui->pathLineEdit->text();
    //record.status = ui->statusCheckBox->isChecked()?1:0;
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
        MessageDialog::error(this,tr("Path required"));
        return false;
    }
    return true;
}


}
