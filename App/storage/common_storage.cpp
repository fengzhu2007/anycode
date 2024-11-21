#include "common_storage.h"
#include "database_helper.h"
#include <QJsonDocument>

namespace ady {

constexpr const  char  CommonStorage::TABLE_NAME[];
constexpr const  char  CommonStorage::COL_NAME[] ;
constexpr const  char  CommonStorage::COL_VALUE[] ;
constexpr const  char  CommonStorage::COL_GROUP[] ;

const QString CommonStorage::VERSION = QLatin1String("version");
const QString CommonStorage::OPTIONS = QLatin1String("options");



CommonStorage::CommonStorage(){

}

CommonRecord CommonStorage::one(const QString& name){
    CommonRecord record;
    QString sql = QString("SELECT * FROM [%1] WHERE [%2]=?").arg(TABLE_NAME).arg(COL_NAME);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,name);
    if(query.exec() && query.next()){
        record = toRecord(query);
    }
    return record;
}



bool CommonStorage::update(const CommonRecord& record)
{
    QString sql = QString("UPDATE [%1] SET [%2]=?,[%3]=? WHERE [%4]=?").arg(TABLE_NAME).arg(COL_VALUE).arg(COL_GROUP).arg(COL_NAME);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,record.value);
    query.bindValue(1,record.group);
    query.bindValue(2,record.name);
    return query.exec();
}

bool CommonStorage::insert(const CommonRecord& record)
{
    QString sql = QString("INSERT INTO [%1] ([%2],[%3],[%4]) VALUES (?,?,?)").arg(TABLE_NAME).arg(COL_NAME).arg(COL_VALUE).arg(COL_GROUP);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,record.name);
    query.bindValue(1,record.value);
    query.bindValue(2,record.group);
    bool ret = query.exec();
    return ret;
}

bool CommonStorage::replace(const CommonRecord& record){
    auto r = this->one(record.name);
    if(r.name.isEmpty()){
       //insert
        return this->insert(record);
    }else{
        return this->update(record);
    }
}

bool CommonStorage::replace(const QString& name,const QString& value){
    return this->replace({name,value,{}});
}

bool CommonStorage::del(const QString& name)
{
    QString sql = QString("DELETE FROM [%1] WHERE [%2]=?").arg(TABLE_NAME).arg(COL_NAME);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,name);
    return query.exec();
}

CommonRecord CommonStorage::toRecord(QSqlQuery& query)
{
    CommonRecord record;
    record.name = query.value(COL_NAME).toString();
    record.value = query.value(COL_VALUE).toString();
    record.group = query.value(COL_GROUP).toLongLong();
    return record;
}


QJsonObject CommonStorage::options(){
    auto r = this->one(OPTIONS);
    if(r.value.isEmpty()){
        return {};
    }else{
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(r.value.toUtf8(),&error);
        if(error.error==QJsonParseError::NoError){
            return doc.object();
        }else{
            return {};
        }
    }
}


}
