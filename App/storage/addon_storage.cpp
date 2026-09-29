#include "addon_storage.h"
#include "database_helper.h"
#include <QVariant>
#include <QDebug>
#include <QDateTime>
namespace ady {
constexpr const  char AddonStorage::TABLE_NAME[];
constexpr const  char AddonStorage::COL_TITLE[];
constexpr const  char AddonStorage::COL_TYPENAME[] ;
constexpr const  char AddonStorage::COL_TYPELABEL[];
constexpr const  char AddonStorage::COL_FILE[] ;
constexpr const  char AddonStorage::COL_EXPORT_TYPE[] ;
constexpr const  char AddonStorage::COL_STATUS[] ;
constexpr const  char AddonStorage::COL_IS_SYSTEM[] ;
constexpr const  char AddonStorage::COL_DESCRIPTION[] ;
constexpr const  char AddonStorage::COL_AUTHOR[] ;
constexpr const  char AddonStorage::COL_VERSION[] ;
AddonStorage::AddonStorage()
{

}

AddonRecord AddonStorage::one(long long id)
{
    AddonRecord record;
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

AddonRecord AddonStorage::one(QString name){
    AddonRecord record;
    QString sql = QString("SELECT * FROM [%1] WHERE [%2]=?").arg(TABLE_NAME).arg(AddonStorage::COL_TYPENAME);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,name);
    bool ret = query.exec();
    this->error = query.lastError();
    if(ret&& query.next()){
        record = toRecord(query);
    }else{
        record.id = 0l;
    }
    return record;
}

QList<AddonRecord> AddonStorage::all()
{
    QList<AddonRecord> lists;
    QString sql = QString("SELECT * FROM [%1] WHERE 1  ORDER BY [%2] ASC ").arg(TABLE_NAME).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    bool ret = query.exec(sql);
    this->error = query.lastError();
    if(ret){
        while(query.next()){
            AddonRecord record = this->toRecord(query);
            lists.push_back(record);
        }
    }
    return lists;
}

QList<AddonRecord> AddonStorage::list(int status)
{
    QList<AddonRecord> lists;
    QString sql = QString("SELECT * FROM [%1] WHERE [%2]=?  ORDER BY [%3] ASC ").arg(TABLE_NAME).arg(COL_STATUS).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,status);
    bool ret = query.exec();
    this->error = query.lastError();
    if(ret){
        while(query.next()){
            AddonRecord record = this->toRecord(query);
            lists.push_back(record);
        }
    }else{
        QSqlError error = query.lastError();
        qDebug()<<"error:"<<error.text();
    }
    return lists;
}

bool AddonStorage::update(AddonRecord record)
{
    QString sql = QString("UPDATE [%1] SET [%2]=?,[%3]=?,[%4]=?,[%5]=?,[%6]=?,[%7]=?,[%8]=?,[%9]=?,[%10]=? WHERE [%11]=?").arg(TABLE_NAME).arg(COL_TITLE).arg(COL_TYPENAME).arg(COL_TYPELABEL).arg(COL_STATUS).arg(COL_IS_SYSTEM).arg(COL_EXPORT_TYPE).arg(COL_DESCRIPTION).arg(COL_AUTHOR).arg(COL_VERSION).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,record.title);
    query.bindValue(1,record.name);
    query.bindValue(2,record.label);
    query.bindValue(3,record.status);
    query.bindValue(4,record.is_system);
    query.bindValue(5,record.export_type);
    query.bindValue(6,record.description);
    query.bindValue(7,record.author);
    query.bindValue(8,record.version);
    query.bindValue(9,record.id);
    bool ret = query.exec();
    this->error = query.lastError();
    return ret;
}

long long AddonStorage::insert(AddonRecord record)
{
    QString sql = QString("INSERT INTO [%1] ([%2],[%3],[%4],[%5],[%6],[%7],[%8],[%9],[%10],[%11],[%12]) VALUES (?,?,?,?,?,?,?,?,?,?,?)").arg(TABLE_NAME).arg(COL_TITLE).arg(COL_TYPENAME).arg(COL_TYPELABEL).arg(COL_FILE).arg(COL_STATUS).arg(COL_IS_SYSTEM).arg(COL_EXPORT_TYPE).arg(COL_DESCRIPTION).arg(COL_AUTHOR).arg(COL_VERSION).arg(DatabaseHelper::COL_DATETIME);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,record.title);
    query.bindValue(1,record.name);
    query.bindValue(2,record.label);
    query.bindValue(3,record.file);
    query.bindValue(4,record.status);
    query.bindValue(5,record.is_system);
    query.bindValue(6,record.export_type);
    query.bindValue(7,record.description);
    query.bindValue(8,record.author);
    query.bindValue(9,record.version);
    query.bindValue(10,QDateTime::currentDateTime().toSecsSinceEpoch());
    bool ret = query.exec();
    this->error = query.lastError();
    if(ret){
        return query.lastInsertId().toLongLong();
    }else{
        return 0;
    }
}

bool AddonStorage::del(long long id)
{
    QString sql = QString("DELETE FROM [%1] WHERE [%2]=?").arg(TABLE_NAME).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,id);
    bool ret = query.exec();
    this->error = query.lastError();
    return ret;
}

bool AddonStorage::del(const QString& name){
    QString sql = QString("DELETE FROM [%1] WHERE [%2]=?").arg(TABLE_NAME).arg(COL_TYPENAME);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0,name);
    bool ret = query.exec();
    this->error = query.lastError();
    return ret;
}

QMap<QString,QString> AddonStorage::dictionary()
{
    QList<AddonRecord> lists = this->list(1);
    QMap<QString,QString> data;
    QList<AddonRecord>::iterator iter = lists.begin();
    while(iter!=lists.end()){
        data[(*iter).name] = (*iter).file;
        iter++;
    }
    return data;
}

AddonRecord AddonStorage::toRecord(QSqlQuery& query)
{
    AddonRecord record;
    record.id = query.value(DatabaseHelper::COL_ID).toLongLong();
    record.title = query.value(COL_TITLE).toString();
    record.name = query.value(COL_TYPENAME).toString();
    record.label = query.value(COL_TYPELABEL).toString();
    record.file = query.value(COL_FILE).toString();
    record.status = query.value(COL_STATUS).toInt();
    record.is_system = query.value(COL_IS_SYSTEM).toInt();
    record.export_type = query.value(COL_EXPORT_TYPE).toLongLong();
    record.description = query.value(COL_DESCRIPTION).toString();
    record.author = query.value(COL_AUTHOR).toString();
    record.version = query.value(COL_VERSION).toString();
    return record;
}



}



