#include "s3_form_dir_setting.h"
#include "ui_s3_form_dir_setting.h"
#include "s3_setting_key.h"
#include <QJsonArray>
#include <QJsonObject>

#include <QDebug>
namespace ady {
    S3FormDirSetting::S3FormDirSetting(QWidget* parent)
        :FormPanel(parent),
        ui(new Ui::S3FormDirSetting)
    {
        ui->setupUi(this);
        setWindowTitle(tr("Dir settings"));

        ui->remoteTreeView->setHeaderLabels(QStringList()<<tr("Local")<<tr("Remote"));
    }

    void S3FormDirSetting::initFormData(SiteRecord record)
    {


        SiteSetting settings =  record.data;
        {
            QJsonValue value = settings.get(S3_REMOTE_DIR_MAPPING);
            if(!value.isNull()){
                ui->remoteTreeView->setValue(value);
            }else{
                ui->remoteTreeView->setValue(QJsonValue());
            }
        }


        {
            QJsonValue value = settings.get(S3_FILTER_EXTENSIONS);
            if(!value.isNull()){
                ui->allowExtLineEdit->setText(value.toString());
            }else{
                ui->allowExtLineEdit->setText("");
            }
        }
    }

    bool S3FormDirSetting::validateFormData(SiteRecord& record)
    {
        if(ui->remoteTreeView->isEmpty()==false){
            if(!ui->remoteTreeView->validate()){
                return false;
            }
            QJsonValue value = ui->remoteTreeView->value();
            record.data.set(S3_REMOTE_DIR_MAPPING,value);
        }else{
            record.data.remove(S3_REMOTE_DIR_MAPPING);
        }

        QString exts = ui->allowExtLineEdit->text();
        if(exts.isEmpty()==false){
            //qDebug()<<"ext edit:"<<ui->allowExtLineEdit->text();
            //qDebug()<<"ext edit bool:"<<exts;
            record.data.set(S3_FILTER_EXTENSIONS,exts.toLower());
        }else{
            record.data.remove(S3_FILTER_EXTENSIONS);
        }
        return true;
    }

    bool S3FormDirSetting::isDataChanged()
    {

        return false;
    }

}
