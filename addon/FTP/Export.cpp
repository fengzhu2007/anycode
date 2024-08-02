#include "Export.h"
#include "FTPPanel.h"
#include "FTPFormGeneral.h"
#include "FTPFormDirSetting.h"
#include "network/network_manager.h"
#include "ftp.h"
#include "storage/site_storage.h"
#include "FTPResponse.h"

#include <QDebug>
ady::Panel* getPanel(long long id,QWidget* parent,QString name)
{
    Q_UNUSED(name);
    return new ady::FTPPanel(id,parent);
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
        return new ady::FTPFormGeneral(parent);
    }else if(n==1){
        return new ady::FTPFormDirSetting(parent);
    }
    return nullptr;
}

ady::NetworkRequest* getRequest(QString name)
{
    Q_UNUSED(name);
    return ady::NetworkManager::getInstance()->newRequest<ady::FTP>();
}

int requestConnect(void* ptr)
{
    ady::SiteRecord* record = (ady::SiteRecord*)ptr;
    if(record->type=="FTP"){
        auto ftp = ady::NetworkManager::getInstance()->newRequest<ady::FTP>();
        ftp->setHost(record->host);
        ftp->setPort(record->port);
        ftp->setUsername(record->username);
        ftp->setPassword(record->password);
        /*qDebug()<<"username:"<<record->username;
        qDebug()<<"port:"<<record->port;
        qDebug()<<"host:"<<record->host;
        qDebug()<<"password:"<<record->password;*/

        ady::FTPResponse* response = (ady::FTPResponse*)ftp->link();
        //response->debug();
        bool status = response->status();
        delete response;

        if(status){
            response = (ady::FTPResponse*)ftp->chDir(record->path);
            status = response->status();
            delete response;

            ady::NetworkResponse * response = ftp->unlink();
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
    return ady::NetworkManager::getInstance()->newRequest<ady::FTP>(id);
}
