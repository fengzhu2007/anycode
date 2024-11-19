#include "oss_form_dir_setting.h"
#include "ui_oss_form_dir_setting.h"
#include "oss_setting_key.h"
#include <QJsonArray>
#include <QJsonObject>

#include <QDebug>
namespace ady {
    OSSFormDirSetting::OSSFormDirSetting(QWidget* parent)
        :FormPanel(parent),
        ui(new Ui::OSSFormDirSettingUi)
    {
        ui->setupUi(this);
        setWindowTitle(tr("Dir settings"));

        ui->remoteTreeView->setHeaderLabels(QStringList()<<tr("Local")<<tr("Remote"));
    }

    void OSSFormDirSetting::initFormData(SiteRecord record)
    {
        SiteSetting settings =  record.data;


        {
            QJsonValue value = settings.get(OSS_REMOTE_DIR_MAPPING);
            if(!value.isNull()){
                 ui->remoteTreeView->setValue(value);
            }else{
                ui->remoteTreeView->setValue(QJsonValue());
            }
        }



        {
            QJsonValue value = settings.get(OSS_FILTER_EXTENSIONS);
            if(!value.isNull()){
                 ui->allowExtLineEdit->setText(value.toString());
            }else{
                ui->allowExtLineEdit->setText("");
            }
        }
    }

    bool OSSFormDirSetting::validateFormData(SiteRecord& record)
    {

        if(ui->remoteTreeView->isEmpty()==false){
            if(!ui->remoteTreeView->validate()){
                return false;
            }
            QJsonValue value = ui->remoteTreeView->value();
            record.data.set(OSS_REMOTE_DIR_MAPPING,value);
        }else{
            record.data.remove(OSS_REMOTE_DIR_MAPPING);
        }

        QString exts = ui->allowExtLineEdit->text();
        if(exts.isEmpty()==false){
             //qDebug()<<"ext edit:"<<ui->allowExtLineEdit->text();
            //qDebug()<<"ext edit bool:"<<exts;
            record.data.set(OSS_FILTER_EXTENSIONS,exts.toLower());
        }else{
            record.data.remove(OSS_FILTER_EXTENSIONS);
        }
        return true;
    }

    bool OSSFormDirSetting::isDataChanged()
    {

        return false;
    }

}
