#include "ftp_form_dir_setting.h"
#include "ui_ftp_form_dir_setting.h"
#include "ftp_setting_key.h"
#include <QJsonArray>
#include <QJsonObject>

#include <QDebug>
namespace ady {
    FTPFormDirSetting::FTPFormDirSetting(QWidget* parent)
        :FormPanel(parent),
        ui(new Ui::FTPFormDirSettingUi)
    {
        ui->setupUi(this);
        setWindowTitle(tr("Dir settings"));

        //ui->localTreeView->setHeaderLabels(QStringList()<<tr("Local")<<tr("Sync Dir"));
        ui->remoteTreeView->setHeaderLabels(QStringList()<<tr("Local")<<tr("Remote"));
    }

    void FTPFormDirSetting::initFormData(SiteRecord record)
    {
        SiteSetting settings =  record.data;


        {
            QJsonValue value = settings.get(FTP_REMOTE_DIR_MAPPING);
            if(!value.isNull()){
                 ui->remoteTreeView->setValue(value);
            }else{
                ui->remoteTreeView->setValue(QJsonValue());
            }
        }
    }

    bool FTPFormDirSetting::validateFormData(SiteRecord& record)
    {

        if(ui->remoteTreeView->isEmpty()==false){
            if(!ui->remoteTreeView->validate()){
                return false;
            }
            QJsonValue value = ui->remoteTreeView->value();
            record.data.set(FTP_REMOTE_DIR_MAPPING,value);
        }else{
            record.data.remove(FTP_REMOTE_DIR_MAPPING);
        }
        return true;
    }

    bool FTPFormDirSetting::isDataChanged()
    {

        return false;
    }

}
