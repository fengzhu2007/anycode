#include "SiteStorage.h"
#include "DatabaseHelper.h"
#include <QVariant>
#include <QSqlQuery>
#include <QDebug>
namespace ady {
constexpr const  char SiteStorage::TABLE_NAME[] ;
constexpr const  char SiteStorage::COL_NAME[];
constexpr const  char SiteStorage::COL_HOST[];
constexpr const  char SiteStorage::COL_PORT[];
constexpr const  char SiteStorage::COL_USERNAME[];
constexpr const  char SiteStorage::COL_PASSWORD[];
constexpr const char SiteStorage::COL_PATH[];
constexpr const  char SiteStorage::COL_PID[] ;
constexpr const  char SiteStorage::COL_TYPE[] ;
constexpr const  char SiteStorage::COL_GROUPID[];
constexpr const  char SiteStorage::COL_STATUS[] ;
constexpr const  char SiteStorage::COL_LISTORDER[];
constexpr const  char SiteStorage::COL_SETTINGS[] ;

SiteStorage::SiteStorage(){

}

SiteRecord SiteStorage::one(long long id){
    SiteRecord record;
    QString sql = QString("SELECT * FROM [%1] WHERE [%2]=?").arg(TABLE_NAME).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,id);
    bool ret = query.exec();
    this->error = query.lastError();
    if(ret && query.next()){
        record = toRecord(query);
    }else{
        record.id = 0l;
    }
    return record;
}

QList<SiteRecord> SiteStorage::all(){
    QList<SiteRecord> lists;
    QString sql = QString("SELECT * FROM [%1] WHERE 1 ORDER BY [%2] DESC").arg(TABLE_NAME).arg(COL_LISTORDER);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    bool ret = query.exec(sql);
    this->error = query.lastError();
    if(ret){
        while(query.next()){
            SiteRecord record = this->toRecord(query);
            lists.push_back(record);
        }
    }
    return lists;
}

QList<SiteRecord> SiteStorage::list(long long pid)
{
    QList<SiteRecord> lists;
    QString sql = QString("SELECT * FROM [%1] WHERE [%2]=? ORDER BY [%3] DESC ").arg(TABLE_NAME).arg(COL_PID).arg(COL_LISTORDER);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,pid);
    bool ret = query.exec();
    this->error = query.lastError();
    if(ret){
        while(query.next()){
            SiteRecord record = this->toRecord(query);
            lists.push_back(record);
        }
    }
    return lists;
}

QList<SiteRecord> SiteStorage::list(long long pid,int status)
{
    QList<SiteRecord> lists;
    QString sql = QString("SELECT * FROM [%1] WHERE [%2]=? AND [%3]=? ORDER BY [%4] DESC ").arg(TABLE_NAME).arg(COL_PID).arg(COL_STATUS).arg(COL_LISTORDER);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,pid);
    query.bindValue(1,status);
    bool ret = query.exec();
    this->error = query.lastError();
    if(ret){
        while(query.next()){
            SiteRecord record = this->toRecord(query);
            lists.push_back(record);
        }
    }
    return lists;
}

bool SiteStorage::update(SiteRecord record)
{
    record.settings = record.data.toJSON();
    QString sql = QString("UPDATE [%1] SET [%2]=?,[%3]=?,[%4]=?,[%5]=?,[%6]=?,[%7]=?,[%8]=?,[%9]=?,[%10]=?,[%11]=?,[%12]=?,[%13]=? WHERE [%14]=?;").arg(TABLE_NAME).arg(COL_NAME).arg(COL_HOST).arg(COL_PORT).arg(COL_USERNAME).arg(COL_PASSWORD).arg(COL_PATH).arg(COL_PID).arg(COL_TYPE).arg(COL_GROUPID).arg(COL_STATUS).arg(COL_LISTORDER).arg(COL_SETTINGS).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,record.name);
    query.bindValue(1,record.host);
    query.bindValue(2,record.port);
    query.bindValue(3,record.username);
    query.bindValue(4,record.password);
    query.bindValue(5,record.path);
    query.bindValue(6,record.pid);
    query.bindValue(7,record.type);
    query.bindValue(8,record.groupid);
    query.bindValue(9,record.status);
    query.bindValue(10,record.listorder);
    query.bindValue(11,record.settings);
    query.bindValue(12,record.id);
    bool ret = query.exec();
    this->error = query.lastError();
    return ret;
}

long long SiteStorage::insert(SiteRecord record)
{
    if(record.settings.isEmpty()){
        record.settings = record.data.toJSON();
    }
    QString sql = QString("INSERT INTO [%1] ([%2],[%3],[%4],[%5],[%6],[%7],[%8],[%9],[%10],[%11],[%12],[%13],[%14]) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?);").arg(TABLE_NAME).arg(COL_NAME).arg(COL_HOST).arg(COL_PORT).arg(COL_USERNAME).arg(COL_PASSWORD).arg(COL_PATH).arg(COL_PID).arg(COL_TYPE).arg(COL_GROUPID).arg(COL_STATUS).arg(COL_LISTORDER).arg(COL_SETTINGS).arg(DatabaseHelper::COL_DATETIME);
    qDebug()<<"sql:"<<sql;
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,record.name);
    query.bindValue(1,record.host);
    query.bindValue(2,record.port);
    query.bindValue(3,record.username);
    query.bindValue(4,record.password);
    query.bindValue(5,record.path);
    query.bindValue(6,record.pid);
    query.bindValue(7,record.type);
    query.bindValue(8,record.groupid);
    query.bindValue(9,record.status);
    query.bindValue(10,record.listorder);
    query.bindValue(11,record.settings);
    query.bindValue(12,record.datatime);
    bool ret = query.exec();
    this->error = query.lastError();
    qDebug()<<"error:"<<this->error.driverText();
    if(ret){
        return query.lastInsertId().toLongLong();
    }else{
        return 0;
    }
}


bool SiteStorage::del(long long id)
{
    QString sql = QString("DELETE FROM [%1] WHERE [%2]=?").arg(TABLE_NAME).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,id);
    bool ret = query.exec();
    this->error = query.lastError();
    return ret;
}

bool SiteStorage::del_list(long long pid)
{
    QString sql = QString("DELETE FROM [%1] WHERE [%2]=?").arg(TABLE_NAME).arg(COL_PID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,pid);
    bool ret = query.exec();
    this->error = query.lastError();
    return ret;
}

SiteRecord SiteStorage::toRecord(QSqlQuery& query)
{
    SiteRecord record;
    record.id = query.value(DatabaseHelper::COL_ID).toLongLong();
    record.name = query.value(COL_NAME).toString();
    record.host = query.value(COL_HOST).toString();
    record.port = query.value(COL_PORT).toInt();
    record.username = query.value(COL_USERNAME).toString();
    record.password = query.value(COL_PASSWORD).toString();
    record.path = query.value(COL_PATH).toString();
    record.pid = query.value(COL_PID).toLongLong();
    record.type = query.value(COL_TYPE).toString();
    record.groupid = query.value(COL_GROUPID).toLongLong();
    record.status = query.value(COL_STATUS).toInt();
    record.listorder = query.value(COL_LISTORDER).toLongLong();
    record.settings = query.value(COL_SETTINGS).toString();
    record.data.load(record.settings);
    return record;
}
}
