#include "{TPL}FormDirSetting.h"
#include "ui_{TPL}FormDirSetting.h"
#include "{TPL}SettingKey.h"
#include <QJsonArray>
#include <QJsonObject>

#include <QDebug>
namespace ady {
    {TPL}FormDirSetting::{TPL}FormDirSetting(QWidget* parent)
        :FormPanel(parent),
        ui(new Ui::{TPL}FormDirSettingUi)
    {
        ui->setupUi(this);
        setWindowTitle(tr("Dir settings"));

        ui->localTreeView->setHeaderLabels(QStringList()<<tr("Local")<<tr("Sync Dir"));
        ui->remoteTreeView->setHeaderLabels(QStringList()<<tr("Local")<<tr("Remote"));
    }

    void {TPL}FormDirSetting::initFormData(SiteRecord record)
    {
        SiteSetting settings =  record.data;
        {
            QJsonValue value = settings.get({TPL}_LOCAL_DIR_SYNC);
            if(!value.isNull()){
                 ui->localTreeView->setValue(value);
            }else{
                ui->localTreeView->setValue(QJsonValue());
            }
        }

        {
            QJsonValue value = settings.get({TPL}_REMOTE_DIR_MAPPING);
            if(!value.isNull()){
                 ui->remoteTreeView->setValue(value);
            }else{
                ui->remoteTreeView->setValue(QJsonValue());
            }
        }
    }

    bool {TPL}FormDirSetting::validateFormData(SiteRecord& record)
    {
        if(ui->localTreeView->isEmpty()==false){
            if(!ui->localTreeView->validate()){
                return false;
            }
            QJsonValue value = ui->localTreeView->value();
            record.data.set({TPL}_LOCAL_DIR_SYNC,value);
        }else{
            record.data.remove({TPL}_LOCAL_DIR_SYNC);
        }
        if(ui->remoteTreeView->isEmpty()==false){
            if(!ui->remoteTreeView->validate()){
                return false;
            }
            QJsonValue value = ui->remoteTreeView->value();
            record.data.set({TPL}_REMOTE_DIR_MAPPING,value);
        }else{
            record.data.remove({TPL}_REMOTE_DIR_MAPPING);
        }
        return true;
    }

    bool {TPL}FormDirSetting::isDataChanged()
    {

        return false;
    }

}
