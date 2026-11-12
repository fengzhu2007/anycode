#include "export.h"
#include "oss_form_general.h"
#include "oss_form_dir_setting.h"
#include "network/network_manager.h"
#include "oss.h"
#include "storage/site_storage.h"
#include "oss_response.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QObject>

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
        auto oss = ady::NetworkManager::getInstance()->newRequest<ady::OSS>();
        oss->setHost(record->host);
        oss->setPort(record->port);
        oss->setUsername(record->username);
        oss->setPassword(record->password);
        //oss->setDefaultDir(record->path);

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

ady::NetworkRequest* initRequest(long long id){
    auto req = ady::NetworkManager::getInstance()->newRequest<ady::OSS>(id);
    return req;
}

const char* addonInfo()
{
    static QByteArray json;
    if(json.isEmpty()){
        QJsonObject object;
        object.insert("name",QStringLiteral("OSS"));
        object.insert("title",QStringLiteral("Aliyun OSS"));
        object.insert("label",QStringLiteral("OSS"));
        object.insert("file",QStringLiteral("OSS/OSS"));
        object.insert("export_type",QStringLiteral("RemoteConnector"));
        object.insert("version",QStringLiteral("1.0"));
        object.insert("author",QStringLiteral("Official"));
        object.insert("description",QObject::tr("Aliyun OSS storage addon providing file browsing, upload and download."));
        json = QJsonDocument(object).toJson(QJsonDocument::Compact);
    }
    return json.constData();
}
