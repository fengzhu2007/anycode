#ifndef COMMITSTORAGE_H
#define COMMITSTORAGE_H
#include "Storage.h"
#include <QDateTime>
#include "cvs/Commit.h"
namespace ady {
    class DatabaseHelper;
    class ANYENGINE_EXPORT CommitRecord{
    public:
        QString oid;
        unsigned int flags;
        long long datetime;//commit time
    };


    class CommitStorage : public Storage{
    public:
        constexpr const static char TABLE_NAME[] = "commit";
        constexpr const static char COL_OID[] = "oid";
        constexpr const static char COL_FLAG[] = "flags";

        CommitStorage(long long id);
        void setFlag(QList<cvs::Commit> lists,unsigned int flags);
        unsigned int setFlag(cvs::Commit commit,unsigned int flags);
        QList<CommitRecord> lists(int start_time,int end_time);

        bool update(CommitRecord record);
        long long insert(CommitRecord record);
        bool del(QString oid);

    private:
        CommitRecord toRecord(QSqlQuery& query);

    private:
        DatabaseHelper* db;



    };
}
#endif // COMMITSTORAGE_H