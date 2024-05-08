#include "Export.h"
#include "{TPL}Panel.h"
#include "{TPL}FormGeneral.h"
#include "{TPL}FormDirSetting.h"
#include "network/NetworkManager.h"
#include "{tpl}.h"
#include "storage/SiteStorage.h"
#include "{TPL}Response.h"

#include <QDebug>
ady::Panel* getPanel(long long id,QWidget* parent,QString name)
{
    Q_UNUSED(name);
    return new ady::{TPL}Panel(id,parent);
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
        return new ady::{TPL}FormGeneral(parent);
    }else if(n==1){
        return new ady::{TPL}FormDirSetting(parent);
    }
    return nullptr;
}

ady::NetworkRequest* getRequest(QString name)
{
    Q_UNUSED(name);
    return ady::NetworkManager::getInstance()->newRequest<ady::{TPL}>();
}

int requestConnect(void* ptr)
{
    ady::SiteRecord* record = (ady::SiteRecord*)ptr;
    if(record->type=="{TPL}"){
        ady::{TPL}* {tpl} = ady::NetworkManager::getInstance()->newRequest<ady::{TPL}>();
        {tpl}->setHost(record->host);
        {tpl}->setPort(record->port);
        {tpl}->setUsername(record->username);
        {tpl}->setPassword(record->password);
        /*qDebug()<<"username:"<<record->username;
        qDebug()<<"port:"<<record->port;
        qDebug()<<"host:"<<record->host;
        qDebug()<<"password:"<<record->password;*/

        ady::{TPL}Response* response = (ady::{TPL}Response*){tpl}->link();
        response->debug();
        bool status = response->status();
        delete response;

        if(status){
            response = (ady::{TPL}Response*){tpl}->chDir(record->path);
            status = response->status();
            delete response;

            ady::NetworkResponse * response = {tpl}->unlink();
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
