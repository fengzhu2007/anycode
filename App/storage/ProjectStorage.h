#ifndef PROJECTSTORAGE_H
#define PROJECTSTORAGE_H
#include <QString>
#include <QList>
#include <QSqlQuery>
#include "Storage.h"
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
#endif // PROJECTSTORAGE_H
