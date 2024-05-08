#ifndef FAVORITESTORAGE_H
#define FAVORITESTORAGE_H
#include <QString>
#include <QSqlQuery>
#include "Storage.h"
#include "global.h"

namespace ady {
    class ANYENGINE_EXPORT FavoriteRecord{
    public:
        long long id;
        QString name;
        QString path;
        long long pid;
        long long sid;
        long long listorder;
    };

    class ANYENGINE_EXPORT FavoriteStorage : public Storage{
    public:
        constexpr const static char TABLE_NAME[] = "favorite";
        constexpr const static char COL_NAME[] = "name";
        constexpr const static char COL_PATH[] = "path";
        constexpr const static char COL_PID[] = "pid";
        constexpr const static char COL_SID[] = "sid";
        constexpr const static char COL_LISTORDER[] = "listorder";

        FavoriteStorage();
        FavoriteRecord one(long long id);
        QList<FavoriteRecord> all();
        QList<FavoriteRecord> list(long long pid,bool local=true);
        bool update(FavoriteRecord record);
        long long insert(FavoriteRecord record);
        bool del(long long id);
    private:
        FavoriteRecord toRecord(QSqlQuery& query);


    };
}

#endif // FAVORITESTORAGE_H
