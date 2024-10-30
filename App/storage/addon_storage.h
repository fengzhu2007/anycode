#ifndef ADDON_STORAGE_H
#define ADDON_STORAGE_H
#include <QString>
#include <QSqlQuery>
#include <QMap>
#include "storage.h"
#include "global.h"
namespace ady {
    class ANYENGINE_EXPORT AddonRecord{
    public:
        long long id;
        QString title;
        QString name;
        QString label;
        QString file;
        int status;
        int is_system;
    };


    class ANYENGINE_EXPORT AddonStorage : public Storage{
    public:
        constexpr const static char TABLE_NAME[] = "addon";
        constexpr const static char COL_TITLE[] = "title";
        constexpr const static char COL_TYPENAME[] = "typename";
        constexpr const static char COL_TYPELABEL[] = "typelabel";
        constexpr const static char COL_FILE[] = "file";
        constexpr const static char COL_STATUS[] = "status";
        constexpr const static char COL_IS_SYSTEM[] = "is_system";



        AddonStorage();
        AddonRecord one(long long id);
        AddonRecord one(QString name);
        QList<AddonRecord> all();
        QList<AddonRecord> list(int status);
        bool update(AddonRecord record);
        long long insert(AddonRecord record);
        bool del(long long id);
        QMap<QString,QString> dictionary();
    private:
        AddonRecord toRecord(QSqlQuery& query);


    };
}
#endif // ADDON_STORAGE_H
