#include "FavoriteStorage.h"
#include "DatabaseHelper.h"
#include <QVariant>
namespace ady {
constexpr const  char FavoriteStorage::TABLE_NAME[];
constexpr const  char FavoriteStorage::COL_NAME[] ;
constexpr const  char FavoriteStorage::COL_PATH[] ;
constexpr const  char FavoriteStorage::COL_PID[] ;
constexpr const  char FavoriteStorage::COL_SID[] ;
constexpr const  char FavoriteStorage::COL_LISTORDER[];

FavoriteStorage::FavoriteStorage(){

}

FavoriteRecord FavoriteStorage::one(long long id){
    FavoriteRecord record;
    QString sql = QString("SELECT * FROM [%1] WHERE [%2]=?").arg(TABLE_NAME).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,id);
    if(query.exec() && query.next()){
        record = toRecord(query);
    }else{
        record.id = 0l;
    }
    return record;
}

QList<FavoriteRecord> FavoriteStorage::all(){
    QList<FavoriteRecord> lists;
    QString sql = QString("SELECT * FROM [%1] WHERE 1  ORDER BY [%2] DESC ").arg(TABLE_NAME).arg(COL_LISTORDER);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    if(query.exec(sql)){
        while(query.next()){
            FavoriteRecord record = this->toRecord(query);
            lists.push_back(record);
        }
    }
    return lists;
}

QList<FavoriteRecord> FavoriteStorage::list(long long pid,bool local)
{
    QList<FavoriteRecord> lists;
    QString sql = QString("SELECT * FROM [%1] WHERE [%2]=? AND [%3]=0 ORDER BY [%4] DESC ").arg(TABLE_NAME).arg(local?COL_PID:COL_SID).arg(local?COL_SID:COL_PID).arg(COL_LISTORDER);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,pid);
    if(query.exec()){
        while(query.next()){
            FavoriteRecord record = this->toRecord(query);
            lists.push_back(record);
        }
    }
    return lists;
}

bool FavoriteStorage::update(FavoriteRecord record)
{
    QString sql = QString("UPDATE [%1] SET [%2]=?,[%3]=?,[%4]=?,[%5]=?,[%6]=? WHERE [%7]=?").arg(TABLE_NAME).arg(COL_NAME).arg(COL_PID).arg(COL_LISTORDER).arg(COL_PATH).arg(COL_SID).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,record.name);
    query.bindValue(1,record.pid);
    query.bindValue(2,record.listorder);
    query.bindValue(3,record.path);
    query.bindValue(4,record.sid);
    query.bindValue(5,record.id);
    return query.exec();
}

long long FavoriteStorage::insert(FavoriteRecord record)
{
    QString sql = QString("INSERT INTO [%1] ([%2],[%3],[%4],[%5],[%6]) VALUES (?,?,?,?,?)").arg(TABLE_NAME).arg(COL_NAME).arg(COL_PID).arg(COL_LISTORDER).arg(COL_PATH).arg(COL_SID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,record.name);
    query.bindValue(1,record.pid);
    query.bindValue(2,record.listorder);
    query.bindValue(3,record.path);
    query.bindValue(4,record.sid);
    bool ret = query.exec();
    if(ret){
        return query.lastInsertId().toLongLong();
    }else{
        return 0;
    }
}


bool FavoriteStorage::del(long long id)
{
    QString sql = QString("DELETE FROM [%1] WHERE [%2]=?").arg(TABLE_NAME).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,id);
    return query.exec();
}

FavoriteRecord FavoriteStorage::toRecord(QSqlQuery& query)
{
    FavoriteRecord record;
    record.id = query.value(DatabaseHelper::COL_ID).toLongLong();
    record.name = query.value(COL_NAME).toString();
    record.pid = query.value(COL_PID).toLongLong();
    record.listorder = query.value(COL_LISTORDER).toLongLong();
    record.path = query.value(COL_PATH).toString();
    record.sid = query.value(COL_SID).toLongLong();
    return record;
}
}
