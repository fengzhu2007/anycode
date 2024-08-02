#ifndef GROUP_STORAGE_H
#define GROUP_STORAGE_H
#include <QString>
#include <QSqlQuery>
#include "Storage.h"
#include "global.h"

namespace ady {
    class ANYENGINE_EXPORT GroupRecord{
    public:
        long long id;
        QString name;
        long long pid;
        long long listorder;
    };

    class ANYENGINE_EXPORT GroupStorage : public Storage{
    public:
        constexpr const static char TABLE_NAME[] = "grouplist";
        constexpr const static char COL_NAME[] = "name";
        constexpr const static char COL_PID[] = "pid";
        constexpr const static char COL_LISTORDER[] = "listorder";

        GroupStorage();
        GroupRecord one(long long id);
        QList<GroupRecord> all();
        QList<GroupRecord> list(long long pid);
        bool update(GroupRecord record);
        long long insert(GroupRecord record);
        bool del(long long id);
    private:
        GroupRecord toRecord(QSqlQuery& query);


    };
}
#endif // GROUP_STORAGE_H
