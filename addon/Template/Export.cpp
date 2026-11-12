#include "export.h"
#include "{tpl}_form_general.h"
#include "{tpl}_form_dir_setting.h"
#include "network/network_manager.h"
#include "{tpl}.h"
#include "storage/site_storage.h"
#include "{tpl}_response.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QObject>

const char* addonInfo()
{
    static QByteArray json;
    if(json.isEmpty()){
        QJsonObject object;
        object.insert("name",QStringLiteral("{TPL}"));
        object.insert("title",QStringLiteral("{TPL}"));
        object.insert("label",QStringLiteral("{TPL}"));
        object.insert("file",QStringLiteral("{TPL}/{TPL}"));
        object.insert("export_type",QStringLiteral("RemoteConnector"));
        object.insert("version",QStringLiteral("1.0"));
        object.insert("author",QStringLiteral("Official"));
        object.insert("description",QObject::tr("{TPL} site addon providing file browsing, upload and download."));
        json = QJsonDocument(object).toJson(QJsonDocument::Compact);
    }
    return json.constData();
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

GetMenusFun getGetMenus()
{
    return &getMenus;
}

QList<ady::MenuData> getMenus(QWidget* parent)
{
    QList<ady::MenuData> menus;

    //add an action to the "Extend" top menu
    ady::MenuData data;
    data.menu_kind = ady::AddonLoader::Extend;
    data.position = ady::AddonLoader::After;
    data.kind = ady::MenuData::Action;
    data.ptr.action = new QAction(QString::fromUtf8("{TPL}"),parent);
    menus.append(data);

    return menus;
}
