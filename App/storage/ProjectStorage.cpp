#include "ProjectStorage.h"
#include "DatabaseHelper.h"
#include <QVariant>

#include <QDebug>
#include <QMessageBox>
namespace ady {
constexpr const  char ProjectStorage::TABLE_NAME[];
constexpr const  char ProjectStorage::COL_NAME[] ;
constexpr const  char ProjectStorage::COL_PATH[] ;
constexpr const  char ProjectStorage::COL_UPDATETIME[] ;
constexpr const  char ProjectStorage::COL_CVS[];//svn , git
constexpr const  char ProjectStorage::COL_CVS_URL[];
constexpr const  char ProjectStorage::COL_CVS_USERNAME[];
constexpr const  char ProjectStorage::COL_CVS_PASSWORD[];


ProjectStorage::ProjectStorage(){

}

ProjectRecord ProjectStorage::one(long long id){
    ProjectRecord record;
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

QList<ProjectRecord> ProjectStorage::all(){
    QList<ProjectRecord> lists;
    QString sql = QString("SELECT * FROM [%1] WHERE 1").arg(TABLE_NAME);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    bool ret = query.exec(sql);
    this->error = query.lastError();

    if(ret){
        while(query.next()){
            ProjectRecord record = this->toRecord(query);
            lists.push_back(record);
        }
    }
    return lists;
}

bool ProjectStorage::update(ProjectRecord record)
{
    QString sql = QString("UPDATE [%1] SET [%2]=?,[%3]=?,[%4]=? WHERE [%5]=?").arg(TABLE_NAME).arg(COL_NAME).arg(COL_PATH).arg(COL_UPDATETIME).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,record.name);
    query.bindValue(1,record.path);
    query.bindValue(2,record.updatetime);
    query.bindValue(3,record.id);
    bool ret = query.exec();
    this->error = query.lastError();
    return ret;
}

long long ProjectStorage::insert(ProjectRecord record)
{
    QString sql = QString("INSERT INTO [%1] ([%2],[%3],[%4],[%5]) VALUES (?,?,?,?)").arg(TABLE_NAME).arg(COL_NAME).arg(COL_PATH).arg(DatabaseHelper::COL_DATETIME).arg(COL_PATH);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,record.name);
    query.bindValue(1,record.path);
    query.bindValue(2,record.datetime);
    query.bindValue(3,record.updatetime);
    bool ret =  query.exec();
    this->error = query.lastError();
    if(ret){
        return query.lastInsertId().toLongLong();
    }else{
        return 0;
    }
}


bool ProjectStorage::del(long long id)
{
    QString sql = QString("DELETE FROM [%1] WHERE [%2]=?").arg(TABLE_NAME).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,id);
    bool ret = query.exec();
    this->error = query.lastError();
    return ret;
}

ProjectRecord ProjectStorage::toRecord(QSqlQuery& query)
{
    ProjectRecord record;
    record.id = query.value(DatabaseHelper::COL_ID).toLongLong();
    record.name = query.value(COL_NAME).toString();
    record.path = query.value(COL_PATH).toString();
    record.datetime = query.value(DatabaseHelper::COL_DATETIME).toLongLong();
    record.updatetime = query.value(COL_PATH).toLongLong();
    return record;
}

}
