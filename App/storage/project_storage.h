#ifndef PROJECT_STORAGE_H
#define PROJECT_STORAGE_H
#include <QString>
#include <QList>
#include <QSqlQuery>
#include <QJsonObject>
#include "storage.h"
#include "global.h"
namespace ady {
    class ANYENGINE_EXPORT ProjectRecord{
    public:
        long long id=0;
        QString name;
        QString path;
        long long datetime=0;
        long long updatetime=0;
        QString cvs;
        QString cvs_url;
        QString cvs_username;
        QString cvs_password;
        QJsonObject toJson(){
            return {
                {"id",id},
                {"name",name},
                {"path",path},
                {"datetime",datetime},
                {"updatetime",updatetime},
                {"cvs",cvs},
                {"cvs_url",cvs_url},
                {"cvs_username",cvs_username},
                {"cvs_password",cvs_password},
            };
        }

        void fromJson(const QJsonObject& json){
            this->id = json.find("id")->toVariant().toLongLong();
            this->name = json.find("name")->toString();
            this->path = json.find("path")->toString();
            this->cvs = json.find("cvs")->toString();
            this->cvs_url = json.find("cvs_url")->toString();
            this->cvs_username = json.find("cvs_username")->toString();
            this->cvs_password = json.find("cvs_password")->toString();
            this->updatetime = json.find("updatetime")->toVariant().toLongLong();
            this->datetime = json.find("datetime")->toVariant().toLongLong();
        }
    };

    class ANYENGINE_EXPORT ProjectStorage : public Storage{
    public:
        constexpr const static char TABLE_NAME[] = "project";
        constexpr const static char COL_NAME[] = "name";
        constexpr const static char COL_PATH[] = "path";
        constexpr const static char COL_UPDATETIME[] = "updatetime";
        constexpr const static char COL_CVS[] = "cvs";
        constexpr const static char COL_CVS_URL[] = "cvs_url";
        constexpr const static char COL_CVS_USERNAME[] = "cvs_username";
        constexpr const static char COL_CVS_PASSWORD[] = "cvs_password";

        ProjectStorage();
        ProjectRecord one(long long id);
        QList<ProjectRecord> all();
        bool update(ProjectRecord record);
        long long insert(ProjectRecord record);
        bool del(long long id);
    private:
        ProjectRecord toRecord(QSqlQuery& query);


    };
}
#endif // PROJECT_STORAGE_H
