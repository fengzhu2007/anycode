#include "group_storage.h"
#include "DatabaseHelper.h"
#include <QVariant>
namespace ady {
constexpr const  char GroupStorage::TABLE_NAME[];
constexpr const  char GroupStorage::COL_NAME[] ;
constexpr const  char GroupStorage::COL_PID[] ;
constexpr const  char GroupStorage::COL_LISTORDER[];

GroupStorage::GroupStorage(){

}

GroupRecord GroupStorage::one(long long id){
    GroupRecord record;
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

QList<GroupRecord> GroupStorage::all(){
    QList<GroupRecord> lists;
    QString sql = QString("SELECT * FROM [%1] WHERE 1  ORDER BY [%2] DESC ").arg(TABLE_NAME).arg(COL_LISTORDER);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    if(query.exec(sql)){
        while(query.next()){
            GroupRecord record = this->toRecord(query);
            lists.push_back(record);
        }
    }
    return lists;
}

QList<GroupRecord> GroupStorage::list(long long pid)
{
    QList<GroupRecord> lists;
    QString sql = QString("SELECT * FROM [%1] WHERE [%2]=0 OR [%3]=? ORDER BY [%4] DESC ").arg(TABLE_NAME).arg(COL_PID).arg(COL_PID).arg(COL_LISTORDER);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,pid);
    if(query.exec()){
        while(query.next()){
            GroupRecord record = this->toRecord(query);
            lists.push_back(record);
        }
    }
    return lists;
}

bool GroupStorage::update(GroupRecord record)
{
    QString sql = QString("UPDATE [%1] SET [%2]=?,[%3]=?,[%4]=? WHERE [%5]=?").arg(TABLE_NAME).arg(COL_NAME).arg(COL_PID).arg(COL_LISTORDER).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,record.name);
    query.bindValue(1,record.pid);
    query.bindValue(2,record.listorder);
    query.bindValue(3,record.id);
    return query.exec();
}

long long GroupStorage::insert(GroupRecord record)
{
    QString sql = QString("INSERT INTO [%1] ([%2],[%3],[%4]) VALUES (?,?,?)").arg(TABLE_NAME).arg(COL_NAME).arg(COL_PID).arg(COL_LISTORDER);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,record.name);
    query.bindValue(1,record.pid);
    query.bindValue(2,record.listorder);
    bool ret = query.exec();
    if(ret){
        return query.lastInsertId().toLongLong();
    }else{
        return 0;
    }
}


bool GroupStorage::del(long long id)
{
    QString sql = QString("DELETE FROM [%1] WHERE [%2]=?").arg(TABLE_NAME).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,id);
    return query.exec();
}

GroupRecord GroupStorage::toRecord(QSqlQuery& query)
{
    GroupRecord record;
    record.id = query.value(DatabaseHelper::COL_ID).toLongLong();
    record.name = query.value(COL_NAME).toString();
    record.pid = query.value(COL_PID).toLongLong();
    record.listorder = query.value(COL_LISTORDER).toLongLong();
    return record;
}
}
