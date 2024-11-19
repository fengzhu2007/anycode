#include "sftp_form_dir_setting.h"
#include "ui_sftp_form_dir_setting.h"
#include "sftp_setting_key.h"
#include <QJsonArray>
#include <QJsonObject>

#include <QDebug>
namespace ady {
    SFTPFormDirSetting::SFTPFormDirSetting(QWidget* parent)
        :FormPanel(parent),
        ui(new Ui::SFTPFormDirSettingUi)
    {
        ui->setupUi(this);
        setWindowTitle(tr("Dir settings"));

        ui->remoteTreeView->setHeaderLabels(QStringList()<<tr("Local")<<tr("Remote"));
    }

    void SFTPFormDirSetting::initFormData(SiteRecord record)
    {
        SiteSetting settings =  record.data;


        {
            QJsonValue value = settings.get(SFTP_REMOTE_DIR_MAPPING);
            if(!value.isNull()){
                 ui->remoteTreeView->setValue(value);
            }else{
                ui->remoteTreeView->setValue(QJsonValue());
            }
        }

        {
            QJsonValue value = settings.get(SFTP_UPLOAD_COMMANDS);
            if(!value.isNull()){
                 ui->commandTextEdit->setText(value.toString());
            }else{
                 ui->commandTextEdit->setText("");
            }
        }
    }

    bool SFTPFormDirSetting::validateFormData(SiteRecord& record)
    {
        if(ui->remoteTreeView->isEmpty()==false){
            if(!ui->remoteTreeView->validate()){
                return false;
            }
            QJsonValue value = ui->remoteTreeView->value();
            record.data.set(SFTP_REMOTE_DIR_MAPPING,value);
        }else{
            record.data.remove(SFTP_REMOTE_DIR_MAPPING);
        }

        QString uploadCommand = ui->commandTextEdit->toPlainText();
        record.data.set(SFTP_UPLOAD_COMMANDS,uploadCommand);

        return true;
    }

    bool SFTPFormDirSetting::isDataChanged()
    {

        return false;
    }

}
