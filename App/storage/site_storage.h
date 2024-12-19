#ifndef SITE_STORAGE_H
#define SITE_STORAGE_H
#include <QString>
#include "site_setting.h"
#include "storage.h"
#include "global.h"
class QSqlQuery;
namespace ady {


    class ANYENGINE_EXPORT SiteRecord
    {
    public:
        long long id = 0;
        QString name;
        QString host;
        int port = 0;
        QString username;
        QString password;
        QString path;
        long long pid = 0;
        QString type;
        long long groupid = 0;
        int status = 1;
        long long listorder = 0;
        QString settings;
        long long datetime = 0;
        SiteSetting data;
        QJsonObject toJson(){
            return {
                    {"id",id},
                    {"name",name},
                    {"host",host},
                    {"port",port},
                    {"username",username},
                    {"password",password},
                    {"path",path},
                    {"pid",pid},
                    {"type",type},
                    {"groupid",groupid},
                {"status",status},
                {"listorder",listorder},
                {"settings",settings},
                {"datetime",datetime},
            };
        }

        void fromJson(const QJsonObject& json){
            this->id = json.find("id")->toVariant().toLongLong();
            this->name = json.find("name")->toString();
            this->host = json.find("host")->toString();
            this->port = json.find("port")->toInt(0);
            this->username = json.find("username")->toString();
            this->password = json.find("password")->toString();
            this->path = json.find("path")->toString();
            this->pid = json.find("pid")->toInt(0);
            this->type = json.find("type")->toString();
            this->groupid = json.find("groupid")->toInt(0);
            this->status = json.find("status")->toInt(0);
            this->listorder = json.find("listorder")->toInt(0);
            this->settings = json.find("settings")->toString();
            this->datetime = json.find("datetime")->toVariant().toLongLong();
        }



    };

    class ANYENGINE_EXPORT SiteStorage  : public Storage
    {
    public:
        constexpr const static char TABLE_NAME[] = "site";
        constexpr const static char COL_NAME[] = "name";
        constexpr const static char COL_HOST[] = "host";
        constexpr const static char COL_PORT[] = "port";
        constexpr const static char COL_USERNAME[] = "username";
        constexpr const static char COL_PASSWORD[] = "password";
        constexpr const static char COL_PATH[] = "path";
        constexpr const static char COL_PID[] = "pid";
        constexpr const static char COL_TYPE[] = "type";
        constexpr const static char COL_GROUPID[] = "groupid";
        constexpr const static char COL_STATUS[] = "status";
        constexpr const static char COL_LISTORDER[] = "listorder";
        constexpr const static char COL_SETTINGS[] = "settings";

        SiteStorage();
        SiteRecord one(long long id,bool igoreCache=false);
        SiteRecord one(long long pid,const QString& host,int port,const QString& username);
        QList<SiteRecord> all();
        QList<SiteRecord> list(long long pid);
        QList<SiteRecord> list(long long pid,int status);
        bool update(SiteRecord record);
        long long insert(SiteRecord record);
        bool del(long long id);
        bool delList(long long pid);
        bool delAll();
    private:
        SiteRecord toRecord(QSqlQuery& query);


    private:
        static QMap<long long,SiteRecord> data;



    };
}
#endif // SITE_STORAGE_H
