#include "Export.h"
#include "OSSPanel.h"
#include "OSSFormGeneral.h"
#include "OSSFormDirSetting.h"
#include "network/NetworkManager.h"
#include "oss.h"
#include "storage/SiteStorage.h"
#include "OSSResponse.h"
#include <QDebug>
ady::Panel* getPanel(long long id,QWidget* parent,QString name)
{
    Q_UNUSED(name);
    return new ady::OSSPanel(id,parent);
}


size_t getFormPanelSize(QString name)
{
    Q_UNUSED(name);
    return 2;
}


ady::FormPanel* getFormPanel(QWidget* parent,QString name,size_t n)
{
    Q_UNUSED(name);
    if(n==0){
        return new ady::OSSFormGeneral(parent);
    }else if(n==1){
        return new ady::OSSFormDirSetting(parent);
    }
    return nullptr;
}

ady::NetworkRequest* getRequest(QString name)
{
    Q_UNUSED(name);
    return ady::NetworkManager::getInstance()->newRequest<ady::OSS>();
}

int requestConnect(void* ptr)
{
    ady::SiteRecord* record = (ady::SiteRecord*)ptr;
    if(record->type=="OSS"){
        ady::OSS* oss = ady::NetworkManager::getInstance()->newRequest<ady::OSS>();
        oss->setHost(record->host);
        oss->setPort(record->port);
        oss->setUsername(record->username);
        oss->setPassword(record->password);
        oss->setDefaultDir(record->path);

        ady::OSSResponse* response = (ady::OSSResponse*)oss->link();
        //response->debug();
        bool status = response->status();
        //int errorCode = response->errorCode;
        int netErrorCode = response->networkErrorCode;
        response->debug();
        delete response;
        if(status){
            return 0;
        }else{
            if(netErrorCode==404){
                return 1;
            }else{
                return 2;//connect failed
            }
        }
    }else{
        return -1;//unknow network type
    }
}
