#ifndef DB_STORAGE_H
#define DB_STORAGE_H
#include <QString>
#include <QSqlQuery>
#include <QMap>
#include "storage.h"
#include "global.h"
#include <QJsonObject>


namespace ady {
class ANYENGINE_EXPORT DBRecord{
public:
    long long id=0l;
    QString name;
    QString driver;
    QString host;
    int port;
    QString username;
    QString password;

    QJsonObject toJson(){
        return {
                {"id",id},
                {"name",name},
                {"driver",driver},
                {"host",host},
                {"port",port},
                {"username",username},
                {"password",password},
        };
    }
    bool isValid(){
        return id>0l;
    }
};

class ANYENGINE_EXPORT DBStorage : public Storage
{
public:
    constexpr const static char  TABLE_NAME[] = "db";
    constexpr const static char  COL_NAME[]   = "name";
    constexpr const static char  COL_DRIVER[]  = "driver";
    constexpr const static char  COL_HOST[]  = "host";//host or file path
    constexpr const static char  COL_PORT[]  = "port";
    constexpr const static char  COL_USERNAME[]  = "username";
    constexpr const static char  COL_PASSWORD[]  = "password";

    DBStorage();
    DBRecord one(long long id);
    QList<DBRecord> all();
    bool update(DBRecord record);
    long long insert(DBRecord record);
    bool del(long long id);
private:
    DBRecord toRecord(QSqlQuery& query);

};

}
#endif // DB_STORAGE_H
