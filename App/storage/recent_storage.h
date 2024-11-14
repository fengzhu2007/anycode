#ifndef RECENT_STORAGE_H
#define RECENT_STORAGE_H
#include <QString>
#include <QList>
#include <QSqlQuery>
#include <QJsonObject>
#include "storage.h"
#include "global.h"
namespace ady {

class ANYENGINE_EXPORT RecentRecord{
public:
    long long id=0;
    long long type=0;
    QString name;
    QString path;
    long long dataid=0;
    long long datetime=0;
    long long updatetime=0;
    QJsonObject toJson(){
        return {
                {"id",id},
                {"type",type},
                {"name",name},
                {"path",path},
                {"dataid",dataid},
                {"datetime",datetime},
                {"updatetime",updatetime},
        };
    }
};

class ANYENGINE_EXPORT RecentStorage : public Storage{
public:
    enum Type{
        File=0,
        ProjectAndFolder
    };
    constexpr const static char TABLE_NAME[] = "recent";
    constexpr const static char COL_TYPE[] = "type";
    constexpr const static char COL_NAME[] = "name";
    constexpr const static char COL_PATH[] = "path";
    constexpr const static char COL_DATAID[] = "dataid";
    constexpr const static char COL_UPDATETIME[] = "updatetime";

    RecentStorage();
    RecentRecord one(long long id);
    QList<RecentRecord> all();
    bool add(Type type,const QString& path);
    bool add(const QString& name,const QString& path,long long dataid);
    QList<RecentRecord> list(Type type,int num=10);
    bool update(RecentRecord record);
    long long insert(RecentRecord record);
    bool del(long long id);
    bool del(Type type,const QString& path);
    bool clear(Type type);
private:
    RecentRecord toRecord(QSqlQuery& query);
};
}


#endif // RECENT_STORAGE_H
