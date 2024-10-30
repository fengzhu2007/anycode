#include "commit_storage.h"
#include "database_helper.h"
#include <QVariant>
#include <QDebug>
namespace ady {
constexpr const  char CommitStorage::TABLE_NAME[] ;
constexpr const  char CommitStorage::COL_OID[] ;
constexpr const  char CommitStorage::COL_FLAG[] ;

    CommitStorage::CommitStorage(long long id)
        :Storage()
    {
        db = DatabaseHelper::getDatabase(id);
    }

    void CommitStorage::setFlag(QList<cvs::Commit> lists,unsigned int flags)
    {
        for(auto commit:lists){
            setFlag(commit,flags);
        }
    }

    unsigned int CommitStorage::setFlag(cvs::Commit commit,unsigned int flags)
    {
        CommitRecord record;
        QString sql = QString("SELECT * FROM [%1] WHERE [%2]=?").arg(TABLE_NAME).arg(COL_OID);
        QSqlQuery query(db->get());
        query.prepare(sql);
        query.bindValue(0,commit.oid());
        bool ret = query.exec();
        this->error = query.lastError();
        if(ret && query.next()){
            record = toRecord(query);
            //UPDATE
            record.flags = flags;
            update(record);
            return record.flags;
        }else{
            //INSERT
            record.oid = commit.oid();
            record.flags = flags;
            record.datetime = commit.time().toSecsSinceEpoch();
            insert(record);
            return record.flags;
        }
    }


    QList<CommitRecord> CommitStorage::lists(int start_time,int end_time)
    {
        QList<CommitRecord> lists;
        QString sql = QString("SELECT * FROM [%1] WHERE [%2]>=? AND [%3]<=?  ORDER BY [%4] DESC ").arg(TABLE_NAME).arg(DatabaseHelper::COL_DATETIME).arg(DatabaseHelper::COL_DATETIME).arg(DatabaseHelper::COL_DATETIME);
        //qDebug()<<"list sql:"<<sql;
        QSqlQuery query(db->get());
        query.prepare(sql);
        query.bindValue(0,start_time);
        query.bindValue(1,end_time);
        bool ret = query.exec();
        this->error = query.lastError();
        if(ret){
            while(query.next()){
                CommitRecord record = this->toRecord(query);
                lists.push_back(record);
            }
        }else{
           this->error = query.lastError();
           qDebug()<<"error:"<<error.text();
        }
        return lists;
    }

    bool CommitStorage::update(CommitRecord record)
    {
        QString sql = QString("UPDATE [%1] SET [%2]=? WHERE [%3]=?").arg(TABLE_NAME).arg(COL_FLAG).arg(COL_OID);
        QSqlQuery query(db->get());
        query.prepare(sql);
        query.bindValue(0,record.flags);
        query.bindValue(1,record.oid);
        bool ret = query.exec();
        this->error = query.lastError();
        if(!ret){
            qDebug()<<"update error:"<<error.text();
        }

        return ret;
    }

    long long CommitStorage::insert(CommitRecord record)
    {
        QString sql = QString("INSERT INTO [%1] ([%2],[%3],[%4]) VALUES (?,?,?)").arg(TABLE_NAME).arg(COL_OID).arg(COL_FLAG).arg(DatabaseHelper::COL_DATETIME);
        QSqlQuery query(db->get());
        query.prepare(sql);
        query.bindValue(0,record.oid);
        query.bindValue(1,record.flags);
        query.bindValue(2,record.datetime);
        bool ret = query.exec();
        this->error = query.lastError();
        if(!ret){
            qDebug()<<"update error:"<<error.text();
        }
        if(ret){

            return query.lastInsertId().toLongLong();
        }else{
            return 0;
        }
    }

    bool CommitStorage::del(QString oid)
    {
        QString sql = QString("DELETE FROM [%1] WHERE [%2]=?").arg(TABLE_NAME).arg(COL_OID);
        QSqlQuery query(db->get());
        query.prepare(sql);
        query.bindValue(0,oid);
        return query.exec();
    }

    bool CommitStorage::delAll(){
        QString sql = QString("DELETE FROM [%1] WHERE 1").arg(TABLE_NAME);
        QSqlQuery query(db->get());
        query.prepare(sql);
        return query.exec();
    }

    CommitRecord CommitStorage::toRecord(QSqlQuery& query)
    {
        CommitRecord record;
        record.oid = query.value(COL_OID).toString();
        record.datetime = query.value(DatabaseHelper::COL_DATETIME).toULongLong();
        record.flags = query.value(COL_FLAG).toInt();
        return record;

    }
}
