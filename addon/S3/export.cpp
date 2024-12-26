#include "export.h"
#include "s3_form_general.h"
#include "s3_form_dir_setting.h"
#include "network/network_manager.h"
#include "s3.h"
#include "storage/site_storage.h"
#include "s3_response.h"
#include <storage/addon_storage.h>

#include <QDebug>

const QString name = QLatin1String("S3");

size_t getFormPanelSize(QString name)
{
    Q_UNUSED(name);
    return 2;
}


ady::FormPanel* getFormPanel(QWidget* parent,QString name,size_t n)
{
    Q_UNUSED(name);
    if(n==0){
        return new ady::S3FormGeneral(parent);
    }else if(n==1){
        return new ady::S3FormDirSetting(parent);
    }
    return nullptr;
}

ady::NetworkRequest* getRequest(QString name)
{
    Q_UNUSED(name);
    return ady::NetworkManager::getInstance()->newRequest<ady::S3>();
}

int requestConnect(void* ptr)
{
    ady::SiteRecord* record = (ady::SiteRecord*)ptr;
    if(record->type==name){
        auto s3 = ady::NetworkManager::getInstance()->newRequest<ady::S3>();
        s3->init(*record);
        ady::S3Response* response = (ady::S3Response*)s3->link();
        response->debug();
        bool status = response->status();
        delete response;

        if(status){
            response = (ady::S3Response*)s3->chDir(record->path);
            status = response->status();
            delete response;

            ady::NetworkResponse * response = s3->unlink();
            delete  response;
            if(status){
                return 0;
            }else{
                return 1;//dir is not exists
            }
        }else{
            //delete response;
            return 2;//connect failed
        }
    }else{
        return -1;//unknow network type
    }
}

ady::NetworkRequest* initRequest(long long id){
    return ady::NetworkManager::getInstance()->newRequest<ady::S3>(id);
}

int install(){
    ady::AddonStorage addonStorage;
    auto r = addonStorage.one(name);
    r.title = QObject::tr("Amazon S3");
    r.name = name;
    r.label = name;
    r.file = name + "/" + name;
    r.status = 1;
    r.is_system = 0;
    r.export_type = 1;
    if(r.id>0l){
        //update
        auto ret = addonStorage.update(r);
        if(!ret){
            return 1;//error
        }
    }else{
        //insert
        auto ret = addonStorage.insert(r);
        if(ret==0l){
            return 1;//error
        }
    }
    return 0;
}

int uninstall(){
    ady::AddonStorage().del(name);
    return 0;
}
