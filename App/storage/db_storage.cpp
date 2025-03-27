#include "db_storage.h"
#include "database_helper.h"
#include <QVariant>
#include <QDebug>

namespace ady{


constexpr const  char DBStorage::TABLE_NAME[];
constexpr const  char DBStorage::COL_NAME[];
constexpr const  char DBStorage::COL_DRIVER[] ;
constexpr const  char DBStorage::COL_HOST[];
constexpr const  char DBStorage::COL_PORT[] ;
constexpr const  char DBStorage::COL_USERNAME[] ;
constexpr const  char DBStorage::COL_PASSWORD[] ;



DBStorage::DBStorage() {

}


DBRecord DBStorage::one(long long id){
    DBRecord record;
    QString sql = QString("SELECT * FROM [%1] WHERE [%2]=?").arg(TABLE_NAME).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,id);
    bool ret = query.exec();
    this->error = query.lastError();
    if(ret&& query.next()){
        record = toRecord(query);
    }else{
        record.id = 0l;
    }
    return record;
}

QList<DBRecord> DBStorage::all(){
    QList<DBRecord> lists;
    QString sql = QString("SELECT * FROM [%1] WHERE 1  ORDER BY [%2] ASC ").arg(TABLE_NAME).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    bool ret = query.exec(sql);
    this->error = query.lastError();
    if(ret){
        while(query.next()){
            DBRecord record = this->toRecord(query);
            lists.push_back(record);
        }
    }
    return lists;
}

bool DBStorage::update(DBRecord record){
    QString sql = QString("UPDATE [%1] SET [%2]=?,[%3]=?,[%4]=?,[%5]=?,[%6]=?,[%7]=? WHERE [%8]=?").arg(TABLE_NAME).arg(COL_NAME).arg(COL_DRIVER).arg(COL_HOST).arg(COL_PORT).arg(COL_USERNAME).arg(COL_PASSWORD).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,record.name);
    query.bindValue(1,record.driver);
    query.bindValue(2,record.host);
    query.bindValue(3,record.port);
    query.bindValue(4,record.username);
    query.bindValue(5,record.password);
    query.bindValue(6,record.id);
    bool ret = query.exec();
    this->error = query.lastError();
    return ret;
}

long long DBStorage::insert(DBRecord record){
    QString sql = QString("INSERT INTO [%1] ([%2],[%3],[%4],[%5],[%6],[%7]) VALUES (?,?,?,?,?,?)").arg(TABLE_NAME).arg(COL_NAME).arg(COL_DRIVER).arg(COL_HOST).arg(COL_PORT).arg(COL_USERNAME).arg(COL_PASSWORD);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,record.name);
    query.bindValue(1,record.driver);
    query.bindValue(2,record.host);
    query.bindValue(3,record.port);
    query.bindValue(4,record.username);
    query.bindValue(5,record.password);
    bool ret = query.exec();
    this->error = query.lastError();
    if(ret){
        return query.lastInsertId().toLongLong();
    }else{
        return 0;
    }
}

bool DBStorage::del(long long id){
    QString sql = QString("DELETE FROM [%1] WHERE [%2]=?").arg(TABLE_NAME).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,id);
    bool ret = query.exec();
    this->error = query.lastError();
    return ret;
}


DBRecord DBStorage::toRecord(QSqlQuery& query)
{
    DBRecord record;
    record.id = query.value(DatabaseHelper::COL_ID).toLongLong();
    record.name = query.value(COL_NAME).toString();
    record.driver = query.value(COL_DRIVER).toString();
    record.host = query.value(COL_HOST).toString();
    record.port = query.value(COL_PORT).toInt();
    record.username = query.value(COL_USERNAME).toString();
    record.password = query.value(COL_PASSWORD).toString();
    return record;
}

}
