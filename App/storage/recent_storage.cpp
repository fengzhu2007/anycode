#include "recent_storage.h"
#include "database_helper.h"
#include <QVariant>
#include <QDebug>

namespace ady {

constexpr const  char RecentStorage::TABLE_NAME[];
constexpr const  char RecentStorage::COL_TYPE[] ;
constexpr const  char RecentStorage::COL_NAME[] ;
constexpr const  char RecentStorage::COL_PATH[] ;
constexpr const  char RecentStorage::COL_DATAID[] ;
constexpr const  char RecentStorage::COL_UPDATETIME[] ;


RecentStorage::RecentStorage(){

}

RecentRecord RecentStorage::one(long long id){
    RecentRecord record;
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

QList<RecentRecord> RecentStorage::all(){
    QList<RecentRecord> lists;
    QString sql = QString("SELECT * FROM [%1] WHERE 1").arg(TABLE_NAME);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    bool ret = query.exec(sql);
    this->error = query.lastError();

    if(ret){
        while(query.next()){
            RecentRecord record = this->toRecord(query);
            lists.push_back(record);
        }
    }
    return lists;
}

bool RecentStorage::add(Type type,const QString& path ){

    QString sql = QString("SELECT * FROM [%1] WHERE [%2]=? AND [%3]=?").arg(TABLE_NAME).arg(COL_TYPE).arg(COL_PATH);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,static_cast<long long>(type));
    query.bindValue(1,path);
    bool ret = query.exec();
    this->error = query.lastError();
    if(ret && query.next()){
        RecentRecord record = toRecord(query);
        //update
        record.updatetime = QDateTime::currentDateTime().toSecsSinceEpoch();
        return this->update(record);
    }else{
        long long timestamp = QDateTime::currentDateTime().toSecsSinceEpoch();
        RecentRecord record = {0ll,static_cast<long long>(type),{},path,0ll,timestamp,timestamp};
        long long id = this->insert(record);
        //delete last
        //find
        QString sql = QString::fromUtf8("SELECT * FROM [%1] WHERE [%2]=? ORDER BY [%3] DESC LIMIT 20,1").arg(TABLE_NAME).arg(COL_TYPE).arg(COL_UPDATETIME);
        QSqlQuery query(DatabaseHelper::getDatabase()->get());
        query.prepare(sql);
        query.bindValue(0,static_cast<long long>(type));
        if(query.exec() && query.next()){
            RecentRecord record = toRecord(query);
            QString sql = QString("DELETE FROM [%1] WHERE [%2]=? AND [%3]<?").arg(TABLE_NAME).arg(COL_TYPE).arg(COL_UPDATETIME);
            QSqlQuery query(DatabaseHelper::getDatabase()->get());
            query.prepare(sql);
            query.bindValue(0,static_cast<long long>(type));
             query.bindValue(0,record.updatetime);
            query.exec();
        }
        return id>00l;
    }
}

bool RecentStorage::add(const QString& name,const QString& path,long long dataid){
    QString sql = QString("SELECT * FROM [%1] WHERE [%2]=? AND [%3]=? AND [%4]=?").arg(TABLE_NAME).arg(COL_TYPE).arg(COL_PATH).arg(COL_DATAID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,static_cast<long long>(RecentStorage::ProjectAndFolder));
    query.bindValue(1,path);
    query.bindValue(2,dataid);
    bool ret = query.exec();
    this->error = query.lastError();
    if(ret && query.next()){
        RecentRecord record = toRecord(query);
        //update
        record.name = name;
        record.updatetime = QDateTime::currentDateTime().toSecsSinceEpoch();
        return this->update(record);
    }else{
        long long timestamp = QDateTime::currentDateTime().toSecsSinceEpoch();
        RecentRecord record = {0ll,static_cast<long long>(RecentStorage::ProjectAndFolder),name,path,dataid,timestamp,timestamp};
        long long id = this->insert(record);
        //delete last
        //find
        QString sql = QString::fromUtf8("SELECT * FROM [%1] WHERE [%2]=? ORDER BY [%3] DESC LIMIT 20,1").arg(TABLE_NAME).arg(COL_TYPE).arg(COL_UPDATETIME);
        QSqlQuery query(DatabaseHelper::getDatabase()->get());
        query.prepare(sql);
        query.bindValue(0,static_cast<long long>(RecentStorage::ProjectAndFolder));
        if(query.exec() && query.next()){
            RecentRecord record = toRecord(query);
            QString sql = QString("DELETE FROM [%1] WHERE [%2]=? AND [%3]<?").arg(TABLE_NAME).arg(COL_TYPE).arg(COL_UPDATETIME);
            QSqlQuery query(DatabaseHelper::getDatabase()->get());
            query.prepare(sql);
            query.bindValue(0,static_cast<long long>(RecentStorage::ProjectAndFolder));
            query.bindValue(0,record.updatetime);
            query.exec();
        }
        return id>00l;
    }
}

QList<RecentRecord> RecentStorage::list(Type type,int num){
    QList<RecentRecord> lists;
    QString sql = QString("SELECT * FROM [%1] WHERE [%2]=? ORDER BY [%3] DESC LIMIT %4").arg(TABLE_NAME).arg(COL_TYPE).arg(COL_UPDATETIME).arg(num);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,static_cast<long long>(type));
    bool ret = query.exec();
    this->error = query.lastError();
    if(ret){
        while(query.next()){
            RecentRecord record = this->toRecord(query);
            lists.push_back(record);
        }
    }
    return lists;
}

bool RecentStorage::update(RecentRecord record)
{
    QString sql = QString("UPDATE [%1] SET [%2]=?,[%3]=?,[%4]=?,[%5]=? WHERE [%6]=?").arg(TABLE_NAME).arg(COL_NAME).arg(COL_PATH).arg(COL_UPDATETIME).arg(COL_DATAID).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,record.name);
    query.bindValue(1,record.path);
    query.bindValue(2,record.updatetime);
    query.bindValue(3,record.dataid);
    query.bindValue(4,record.id);
    bool ret = query.exec();
    this->error = query.lastError();
    return ret;
}

long long RecentStorage::insert(RecentRecord record)
{
    QString sql = QString("INSERT INTO [%1] ([%2],[%3],[%4],[%5],[%6],[%7]) VALUES (?,?,?,?,?,?)").arg(TABLE_NAME).arg(COL_TYPE).arg(COL_NAME).arg(COL_PATH).arg(DatabaseHelper::COL_DATETIME).arg(COL_UPDATETIME).arg(COL_DATAID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,record.type);
    query.bindValue(1,record.name);
    query.bindValue(2,record.path);
    query.bindValue(3,record.datetime);
    query.bindValue(4,record.updatetime);
    query.bindValue(5,record.dataid);
    //qDebug()<<"sql:"<<sql;
    bool ret =  query.exec();
    this->error = query.lastError();
    if(ret){
        return query.lastInsertId().toLongLong();
    }else{
        return 0;
    }
}


bool RecentStorage::del(long long id)
{
    QString sql = QString("DELETE FROM [%1] WHERE [%2]=?").arg(TABLE_NAME).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,id);
    bool ret = query.exec();
    this->error = query.lastError();
    return ret;
}

bool RecentStorage::del(Type type,const QString& path){
    QString sql = QString("DELETE FROM [%1] WHERE [%2]=? AND [%3]=?").arg(TABLE_NAME).arg(COL_TYPE).arg(COL_PATH);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,static_cast<long long>(type));
    query.bindValue(1,path);
    bool ret = query.exec();
    this->error = query.lastError();
    return ret;
}

bool RecentStorage::clear(Type type){
    QString sql = QString("DELETE FROM [%1] WHERE [%2]=?").arg(TABLE_NAME).arg(COL_TYPE);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,static_cast<long long>(type));
    bool ret = query.exec();
    this->error = query.lastError();
    return ret;
}

RecentRecord RecentStorage::toRecord(QSqlQuery& query)
{
    RecentRecord record;
    record.id = query.value(DatabaseHelper::COL_ID).toLongLong();
    record.type = query.value(COL_TYPE).toLongLong();
    record.name = query.value(COL_NAME).toString();
    record.path = query.value(COL_PATH).toString();
    record.dataid = query.value(COL_DATAID).toLongLong();
    record.datetime = query.value(DatabaseHelper::COL_DATETIME).toLongLong();
    record.updatetime = query.value(COL_UPDATETIME).toLongLong();
    return record;
}

}

