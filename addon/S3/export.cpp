#include "export.h"
#include "s3_form_general.h"
#include "s3_form_dir_setting.h"
#include "network/network_manager.h"
#include "s3.h"
#include "storage/site_storage.h"
#include "s3_response.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QObject>

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

const char* addonInfo()
{
    static QByteArray json;
    if(json.isEmpty()){
        QJsonObject object;
        object.insert("name",QStringLiteral("S3"));
        object.insert("title",QStringLiteral("Amazon S3"));
        object.insert("label",QStringLiteral("S3"));
        object.insert("file",QStringLiteral("S3/S3"));
        object.insert("export_type",QStringLiteral("RemoteConnector"));
        object.insert("version",QStringLiteral("1.0"));
        object.insert("author",QStringLiteral("Official"));
        object.insert("description",QObject::tr("Amazon S3 storage addon providing file browsing, upload and download."));
        json = QJsonDocument(object).toJson(QJsonDocument::Compact);
    }
    return json.constData();
}
