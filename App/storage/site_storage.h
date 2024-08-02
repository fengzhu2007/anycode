#ifndef SITE_STORAGE_H
#define SITE_STORAGE_H
#include <QString>
#include "SiteSetting.h"
#include "Storage.h"
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
        SiteRecord one(long long id);
        QList<SiteRecord> all();
        QList<SiteRecord> list(long long pid);
        QList<SiteRecord> list(long long pid,int status);
        bool update(SiteRecord record);
        long long insert(SiteRecord record);
        bool del(long long id);
        bool del_list(long long pid);
    private:
        SiteRecord toRecord(QSqlQuery& query);

    };
}
#endif // SITE_STORAGE_H
