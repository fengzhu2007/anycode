#include "Export.h"
#include "COSPanel.h"
#include "COSFormGeneral.h"
#include "COSFormDirSetting.h"
#include "network/network_manager.h"
#include "network/network_request.h"
#include "cos.h"
#include "storage/site_storage.h"
#include "COSResponse.h"

#include <QDebug>
ady::Panel* getPanel(long long id,QWidget* parent,QString name)
{
    Q_UNUSED(name);
    return new ady::COSPanel(id,parent);
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
        return new ady::COSFormGeneral(parent);
    }else if(n==1){
        return new ady::COSFormDirSetting(parent);
    }
    return nullptr;
}

ady::NetworkRequest* getRequest(QString name)
{
    Q_UNUSED(name);
    return ady::NetworkManager::getInstance()->newRequest<ady::COS>();
}

int requestConnect(void* ptr)
{
    ady::SiteRecord* record = (ady::SiteRecord*)ptr;
    if(record->type=="COS"){
        auto cos = ady::NetworkManager::getInstance()->newRequest<ady::COS>();
        cos->setHost(record->host);
        cos->setPort(record->port);
        cos->setUsername(record->username);
        cos->setPassword(record->password);
        /*qDebug()<<"username:"<<record->username;
        qDebug()<<"port:"<<record->port;
        qDebug()<<"host:"<<record->host;
        qDebug()<<"password:"<<record->password;*/

        ady::COSResponse* response = (ady::COSResponse*)cos->link();
        response->debug();
        bool status = response->status();
        delete response;

        if(status){
            response = (ady::COSResponse*)cos->chDir(record->path);
            status = response->status();
            delete response;

            ady::NetworkResponse * response = cos->unlink();
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
    return ady::NetworkManager::getInstance()->newRequest<ady::COS>(id);
}
