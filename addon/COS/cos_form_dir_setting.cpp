#include "cos_form_dir_setting.h"
#include "ui_cos_form_dir_setting.h"
#include "cos_setting_key.h"
#include <QJsonArray>
#include <QJsonObject>

#include <QDebug>
namespace ady {
    COSFormDirSetting::COSFormDirSetting(QWidget* parent)
        :FormPanel(parent),
        ui(new Ui::COSFormDirSettingUi)
    {
        ui->setupUi(this);
        setWindowTitle(tr("Dir settings"));

        //ui->localTreeView->setHeaderLabels(QStringList()<<tr("Local")<<tr("Sync Dir"));
        ui->remoteTreeView->setHeaderLabels(QStringList()<<tr("Local")<<tr("Remote"));
        //ui->allowDirTreeView->setHeaderLabels(QStringList()<<tr("Dirs"));
        //ui->allowDirTreeView->setColumnWidth(0,120);
    }

    void COSFormDirSetting::initFormData(SiteRecord record)
    {
        SiteSetting settings =  record.data;


        {
            QJsonValue value = settings.get(COS_REMOTE_DIR_MAPPING);
            if(!value.isNull()){
                 ui->remoteTreeView->setValue(value);
            }else{
                ui->remoteTreeView->setValue(QJsonValue());
            }
        }


        {
            QJsonValue value = settings.get(COS_FILTER_EXTENSIONS);
            if(!value.isNull()){
                 ui->allowExtLineEdit->setText(value.toString());
            }else{
                ui->allowExtLineEdit->setText("");
            }
        }
    }

    bool COSFormDirSetting::validateFormData(SiteRecord& record)
    {

        if(ui->remoteTreeView->isEmpty()==false){
            if(!ui->remoteTreeView->validate()){
                return false;
            }
            QJsonValue value = ui->remoteTreeView->value();
            record.data.set(COS_REMOTE_DIR_MAPPING,value);
        }else{
            record.data.remove(COS_REMOTE_DIR_MAPPING);
        }

        QString exts = ui->allowExtLineEdit->text();
        if(exts.isEmpty()==false){
             //qDebug()<<"ext edit:"<<ui->allowExtLineEdit->text();
            //qDebug()<<"ext edit bool:"<<exts;
            record.data.set(COS_FILTER_EXTENSIONS,exts.toLower());
        }else{
            record.data.remove(COS_FILTER_EXTENSIONS);
        }
        return true;
    }

    bool COSFormDirSetting::isDataChanged()
    {

        return false;
    }

}
