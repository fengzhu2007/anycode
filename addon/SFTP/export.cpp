#include "export.h"
#include "sftp_form_general.h"
#include "sftp_form_dir_setting.h"
#include "network/network_manager.h"
#include "sftp.h"
#include "storage/site_storage.h"
#include "sftp_response.h"

#include <QDebug>


size_t getFormPanelSize(QString name)
{
    Q_UNUSED(name);
    return 2;
}


ady::FormPanel* getFormPanel(QWidget* parent,QString name,size_t n)
{
    Q_UNUSED(name);
    if(n==0){
        return new ady::SFTPFormGeneral(parent);
    }else if(n==1){
        return new ady::SFTPFormDirSetting(parent);
    }
    return nullptr;
}

ady::NetworkRequest* getRequest(QString name)
{
    Q_UNUSED(name);
    return ady::NetworkManager::getInstance()->newRequest<ady::SFTP>();
}

int requestConnect(void* ptr)
{
    ady::SiteRecord* record = (ady::SiteRecord*)ptr;
    if(record->type=="SFTP"){
        auto sftp = ady::NetworkManager::getInstance()->newRequest<ady::SFTP>();
        sftp->setHost(record->host);
        sftp->setPort(record->port);
        sftp->setUsername(record->username);
        sftp->setPassword(record->password);
        /*qDebug()<<"username:"<<record->username;
        qDebug()<<"port:"<<record->port;
        qDebug()<<"host:"<<record->host;
        qDebug()<<"password:"<<record->password;*/

        ady::SFTPResponse* response = (ady::SFTPResponse*)sftp->link();
        //response->debug();
        bool status = response->status();
        int errorCode = response->errorCode;
        delete response;

        if(status){
            return 0;
            /*response = (ady::SFTPResponse*)sftp->chDir(record->path);
            status = response->status();
            delete response;

            ady::NetworkResponse * response = sftp->unlink();
            delete  response;
            if(status){
                return 0;
            }else{
                return 1;//dir is not exists
            }*/
        }else{
            if(errorCode==78){
                return 1;
            }
            //delete response;
            return 2;//connect failed
        }
    }else{
        return -1;//unknow network type
    }
}

ady::NetworkRequest* initRequest(long long id){
    auto req = ady::NetworkManager::getInstance()->newRequest<ady::SFTP>(id);
    return req;
}
