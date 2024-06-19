#include "DatabaseHelper.h"
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QMessageBox>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include "CommonStorage.h"
#include "AddonStorage.h"
#include "SiteStorage.h"
#include "ProjectStorage.h"
#include "CommitStorage.h"
#include "GroupStorage.h"
#include "FavoriteStorage.h"

#include <QDebug>
namespace ady {
    DatabaseHelper* DatabaseHelper::instance = nullptr;
    QMap<QString,DatabaseHelper*> DatabaseHelper::dbLists;

    constexpr const  char DatabaseHelper::COL_ID[]  ;
    constexpr const  char DatabaseHelper::COL_DATETIME[] ;

    constexpr const  char DatabaseHelper::DBNAME[] ;
    constexpr const  char DatabaseHelper::DBNAMEPRJ[] ;//project database



    DatabaseHelper::DatabaseHelper(QString filename){
        /*QString lists = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if(lists.size()>0){
            QMessageBox::information(nullptr,QObject::tr("SQL Error"),lists[0]);
            QDir dir(lists[0]);
            if(!dir.exists()){
                QString name = dir.dirName();
                dir.cdUp();
                dir.mkdir(name);
                //dir.makeAbsolute()
            }
            this->dbname = lists[0] + "/"+filename;
        }else{
            this->dbname = filename;
        }*/
        this->dbname = filename;

#ifdef Q_OS_MAC
        QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir d(dir);
        if(d.exists()==false){
            QString name = d.dirName();
            d.cdUp();
            d.mkdir(name);
        }
        this->db = QSqlDatabase::addDatabase("QSQLITE",dir + "/"+filename);
        this->db.setDatabaseName(dir + "/"+this->dbname);
        //this->db.setUserName("anycode");
        //this->db.setPassword("123123456");
#else

        QString dir = QCoreApplication::applicationDirPath();
        QString dir_name = "data";
        QDir d(dir);
        if(!d.exists(dir_name)){
            if(d.mkdir(dir_name)){
                d.cd(dir_name);
            }
        }else{
            d.cd(dir_name);
        }
        dir = d.absolutePath();
        //qDebug()<<"db file:"<<dir + "/"+filename;
        this->db = QSqlDatabase::addDatabase("QSQLITE",dir + "/"+filename);
        //qDebug()<<"databaseName:"<<this->db.databaseName();
        this->db.setDatabaseName(dir + "/" + this->dbname);
#endif

        if(!this->open()){
            QMessageBox::information(nullptr,QObject::tr("SQL Error"),QString("%1 create failed").arg(filename));

            //QMessageBox::information(nullptr,QObject::tr("SQL Error"), this->db.lastError().text());
            qDebug()<<"database open failed";
        }else{
            this->onUpgrade();
        }
    }

    DatabaseHelper::~DatabaseHelper()
    {
        this->db.close();
    }


    bool DatabaseHelper::open()
    {
        if(!this->db.isOpen()){

            return this->db.open();
        }else{
            return true;
        }
    }

    bool DatabaseHelper::open(QString username,QString password)
    {
        if(!this->db.isOpen()){
            return this->db.open(username,password);
        }else{
            return true;
        }
    }

    QSqlDatabase DatabaseHelper::get(){
        return this->db;
    }

    int DatabaseHelper::dbType()
    {
        if(this->dbname.startsWith("data")){
            return 1;
        }else if(this->dbname.startsWith("project")){
            return 2;
        }else{
            return 0;
        }
    }

    void DatabaseHelper::close()
    {
        this->db.close();
    }



    bool DatabaseHelper::tableExists(QString tablename)
    {
        QStringList lists = this->db.tables();
        return lists.contains(tablename);
    }

    int DatabaseHelper::getVersion()
    {
        if(this->tableExists(CommonStorage::TABLE_NAME)==false){
            return 0;
        }
        int version = 0;
        QString sql = QString("SELECT [%1] FROM %2 WHERE [%3]=? ").arg(CommonStorage::COL_VALUE).arg(CommonStorage::TABLE_NAME).arg(CommonStorage::COL_NAME);
        QSqlQuery query(this->db);
        query.prepare(sql);
        query.bindValue(0,DatabaseHelper::VERSION);
        if(query.exec()){
            if(query.next()){
                QString value = query.value(0).toString();
                version = value.toInt();
            }
        }
        return version;
    }

    bool DatabaseHelper::setVersion(){

        QString sql = QString("REPLACE INTO [%1] ([%2],[%3],[%4]) VALUES  (?, ?, ?)").arg(CommonStorage::TABLE_NAME).arg(CommonStorage::COL_VALUE).arg(CommonStorage::COL_NAME).arg(CommonStorage::COL_GROUP);

        QSqlQuery query(this->db);
        query.prepare(sql);
        query.addBindValue(DatabaseHelper::VERSIONID);
        query.addBindValue(DatabaseHelper::VERSION);
        query.addBindValue("");
        bool ret = query.exec();
        //QSqlError error = query.lastError();
        //qDebug()<<"sql:"<<sql<<";error:"<<ret<<":"<<error.text();
        return ret;

    }

    void DatabaseHelper::onUpgrade()
    {
        int version = this->getVersion();
        switch(version){
            case 0:
                upgradeV0();
            case 1:
                upgradeV1();
            case 2:
                upgradeV2();
            case 3:
                upgradeV3();
            case 4:
                upgradeV4();
            case 5:
                upgradeV5();
            default:

                break;
        }
        //qDebug()<<"version:"<<version;
        if(version != VERSIONID){
           bool ret = this->setVersion();
        }
    }

    void DatabaseHelper::upgradeV0()
    {

        int type = this->dbType();
        if(type==1){
            //main dbname
            //project
            QString sql = QString("CREATE TABLE [%1] ([%2] INTEGER  NOT NULL PRIMARY KEY AUTOINCREMENT,[%3] VARCHAR(200)  NULL,[%4] VARCHAR(600)  NULL,[%5] INTEGER  NULL)").arg(ProjectStorage::TABLE_NAME).arg(COL_ID).arg(ProjectStorage::COL_NAME).arg(ProjectStorage::COL_PATH).arg(COL_DATETIME);
            this->db.exec(sql);

            //site
            sql = QString("CREATE TABLE [%1] ([%2] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,[%3] VARCHAR(200)  NULL,[%4] VARCHAR(100)  NULL,[%5] INTEGER  NULL,[%6] VARCHAR(100)  NULL,[%7] VARCHAR(100)  NULL,[%8] VARCHAR(250)  NULL,[%9] INTEGER DEFAULT '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''0''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' NULL,[%10] INTEGER  NULL,[%11] INTEGER DEFAULT '''''''''''''''''''''''''''''''0''''''''''''''''''''''''''''''' NULL,[%12] INTEGER DEFAULT '''''''''''''''1''''''''''''''' NULL,[%13] VARCHAR(100)  NULL,[%14] TEXT  NULL,[%15] INTEGER DEFAULT '0' NULL)").arg(SiteStorage::TABLE_NAME).arg(COL_ID).arg(SiteStorage::COL_NAME).arg(SiteStorage::COL_HOST).arg(SiteStorage::COL_PORT).arg(SiteStorage::COL_USERNAME).arg(SiteStorage::COL_PASSWORD).arg(SiteStorage::COL_PATH).arg(SiteStorage::COL_PID).arg(COL_DATETIME).arg(SiteStorage::COL_GROUPID).arg(SiteStorage::COL_STATUS).arg(SiteStorage::COL_TYPE).arg(SiteStorage::COL_SETTINGS).arg(SiteStorage::COL_LISTORDER);
            this->db.exec(sql);


            //common
            sql = QString("CREATE TABLE [%1] ([%2] VARCHAR(100)  UNIQUE NULL PRIMARY KEY,[%3] TEXT  NULL,[%4] VARCHAR(100)  NULL)").arg(CommonStorage::TABLE_NAME).arg(CommonStorage::COL_NAME).arg(CommonStorage::COL_VALUE).arg(CommonStorage::COL_GROUP);
            this->db.exec(sql);

            //group
            sql = QString("CREATE TABLE [%1] ([%2] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,[%3] VARCHAR(250)  NULL,[%4] INTEGER DEFAULT '''''''''''''''''''''''''''''''0''''''''''''''''''''''''''''''' NULL,[%5] INTEGER DEFAULT '0' NULL)").arg(GroupStorage::TABLE_NAME).arg(COL_ID).arg(GroupStorage::COL_NAME).arg(GroupStorage::COL_PID).arg(GroupStorage::COL_LISTORDER);
            this->db.exec(sql);


            {
                QString sql = QString("INSERT INTO [%1] ([%2],[%3],[%4]) VALUES (?,?,?)").arg(GroupStorage::TABLE_NAME).arg(GroupStorage::COL_NAME).arg(GroupStorage::COL_PID).arg(GroupStorage::COL_LISTORDER);
                QSqlQuery query(this->db);
                query.prepare(sql);
                query.bindValue(0, QObject::tr("Development"));
                query.bindValue(1,0);
                query.bindValue(2,999);
                query.exec();

                query.prepare(sql);
                query.bindValue(0, QObject::tr("Production"));
                query.bindValue(1,0);
                query.bindValue(2,998);
                query.exec();

                query.prepare(sql);
                query.bindValue(0, QObject::tr("Other"));
                query.bindValue(1,0);
                query.bindValue(2,997);
                query.exec();
            }

            //addon
            sql = QString("CREATE TABLE [%1] ([%2] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,[%3] VARCHAR(250)  NULL,[%4] VARCHAR(100)  NULL,[%5] VARCHAR(100)  NULL,[%6] VARCHAR(100)  NULL,[%7] INTEGER DEFAULT '''0''' NULL,[%8] INTEGER DEFAULT '''1''' NULL,[%9] INTEGER DEFAULT '0' NULL)").arg(AddonStorage::TABLE_NAME).arg(COL_ID).arg(AddonStorage::COL_TITLE).arg(AddonStorage::COL_TYPENAME).arg(AddonStorage::COL_TYPELABEL).arg(AddonStorage::COL_FILE).arg(COL_DATETIME).arg(AddonStorage::COL_STATUS).arg(AddonStorage::COL_IS_SYSTEM);
            this->db.exec(sql);

            {
                QString sql = QString("INSERT INTO [%1] ([%2],[%3],[%4],[%5],[%6],[%7],[%8]) VALUES (?,?,?,?,?,?,?)").arg(AddonStorage::TABLE_NAME).arg(AddonStorage::COL_TITLE).arg(AddonStorage::COL_TYPENAME).arg(AddonStorage::COL_TYPELABEL).arg(AddonStorage::COL_FILE).arg(AddonStorage::COL_STATUS).arg(AddonStorage::COL_IS_SYSTEM).arg(COL_DATETIME);
                QSqlQuery query(this->db);
                query.prepare(sql);
                query.bindValue(0, "FTP");
                query.bindValue(1, "FTP");
                query.bindValue(2, "FTP");
                query.bindValue(3, "FTP/FTP");
                query.bindValue(4, 1);
                query.bindValue(5, 1);
                query.bindValue(6, QDateTime::currentDateTime().toSecsSinceEpoch());
                query.exec();
                //QSqlError error = query.lastError();
                //qDebug()<<"error:"<<error.text();
            }




        }else if(type==2){

            QString sql = QString("CREATE TABLE [%1] ([%2] VARCHAR(100)  UNIQUE NULL PRIMARY KEY,[%3] TEXT  NULL,[%4] VARCHAR(100)  NULL)").arg(CommonStorage::TABLE_NAME).arg(CommonStorage::COL_NAME).arg(CommonStorage::COL_VALUE).arg(CommonStorage::COL_GROUP);
            this->db.exec(sql);

            //git project dbname
            //commit
            sql = QString("CREATE TABLE [%1] ([%2] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,[%3] VARCHAR(100)  NOT NULL,[%4] INTEGER  NULL,[%5] INTEGER  NULL)").arg(CommitStorage::TABLE_NAME).arg(COL_ID).arg(CommitStorage::COL_OID).arg(CommitStorage::COL_FLAG).arg(COL_DATETIME);
            this->db.exec(sql);

            sql = QString("CREATE UNIQUE INDEX [IDX_COMMIT_OID] ON [%1]([%2] ASC)").arg(CommitStorage::TABLE_NAME).arg(CommitStorage::COL_OID);
            this->db.exec(sql);
        }


    }


    void DatabaseHelper::upgradeV1()
    {
        int type = this->dbType();
        if(type==1){
            {
                QString sql = QString("INSERT INTO [%1] ([%2],[%3],[%4],[%5],[%6],[%7],[%8]) VALUES (?,?,?,?,?,?,?)").arg(AddonStorage::TABLE_NAME).arg(AddonStorage::COL_TITLE).arg(AddonStorage::COL_TYPENAME).arg(AddonStorage::COL_TYPELABEL).arg(AddonStorage::COL_FILE).arg(AddonStorage::COL_STATUS).arg(AddonStorage::COL_IS_SYSTEM).arg(COL_DATETIME);
                QSqlQuery query(this->db);
                query.prepare(sql);
                query.bindValue(0, "OSS");
                query.bindValue(1, "OSS");
                query.bindValue(2, "OSS");
                query.bindValue(3, "OSS/OSS");
                query.bindValue(4, 1);
                query.bindValue(5, 1);
                query.bindValue(6, QDateTime::currentDateTime().toSecsSinceEpoch());
                query.exec();
            }
        }

    }

    void DatabaseHelper::upgradeV2(){
        int type = this->dbType();
        if(type==1){
            {
                QString sql = QString("INSERT INTO [%1] ([%2],[%3],[%4],[%5],[%6],[%7],[%8]) VALUES (?,?,?,?,?,?,?)").arg(AddonStorage::TABLE_NAME).arg(AddonStorage::COL_TITLE).arg(AddonStorage::COL_TYPENAME).arg(AddonStorage::COL_TYPELABEL).arg(AddonStorage::COL_FILE).arg(AddonStorage::COL_STATUS).arg(AddonStorage::COL_IS_SYSTEM).arg(COL_DATETIME);
                QSqlQuery query(this->db);
                query.prepare(sql);
                query.bindValue(0, "SFTP");
                query.bindValue(1, "SFTP");
                query.bindValue(2, "SFTP");
                query.bindValue(3, "SFTP/SFTP");
                query.bindValue(4, 1);
                query.bindValue(5, 1);
                query.bindValue(6, QDateTime::currentDateTime().toSecsSinceEpoch());
                query.exec();
            }

            {
                QString sql = QString("INSERT INTO [%1] ([%2],[%3],[%4],[%5],[%6],[%7],[%8]) VALUES (?,?,?,?,?,?,?)").arg(AddonStorage::TABLE_NAME).arg(AddonStorage::COL_TITLE).arg(AddonStorage::COL_TYPENAME).arg(AddonStorage::COL_TYPELABEL).arg(AddonStorage::COL_FILE).arg(AddonStorage::COL_STATUS).arg(AddonStorage::COL_IS_SYSTEM).arg(COL_DATETIME);
                QSqlQuery query(this->db);
                query.prepare(sql);
                query.bindValue(0, "COS");
                query.bindValue(1, "COS");
                query.bindValue(2, "COS");
                query.bindValue(3, "COS/COS");
                query.bindValue(4, 1);
                query.bindValue(5, 1);
                query.bindValue(6, QDateTime::currentDateTime().toSecsSinceEpoch());
                query.exec();
            }
        }
    }

    void DatabaseHelper::upgradeV3(){
        int type = this->dbType();
        if(type==1){
            QString sql = QString("alter table [%1] add column [%2] INTEGER  NULL").arg(ProjectStorage::TABLE_NAME).arg(ProjectStorage::COL_UPDATETIME);
            this->db.exec(sql);
        }
    }

     void DatabaseHelper::upgradeV4()
     {
         int type = this->dbType();
         if(type==1){

            QString sql = QString("CREATE TABLE [%1] (\
                                  [%2] INTEGER  NOT NULL PRIMARY KEY AUTOINCREMENT,\
                                  [%3] VARCHAR(250)  NULL,\
                                  [%4] VARCHAR(600)  NULL,\
                                  [%5] INTEGER DEFAULT '0' NULL,\
                                  [%6] INTEGER DEFAULT '0' NULL,\
                                  [%7] INTEGER(6) DEFAULT '0' NULL\
                                  )").arg(FavoriteStorage::TABLE_NAME).arg(COL_ID).arg(FavoriteStorage::COL_NAME).arg(FavoriteStorage::COL_PATH).arg(FavoriteStorage::COL_PID).arg(FavoriteStorage::COL_SID).arg(FavoriteStorage::COL_LISTORDER);


            this->db.exec(sql);
         }
     }

     void DatabaseHelper::upgradeV5()
     {
         int type = this->dbType();
         if(type==1){
                QString sql = QString("alter table [%1] add column "
                                      "[%2] VARCHAR(60) NULL,"
                                      "[%3] VARCHAR(250) NULL,"
                                      "[%4] VARCHAR(100) NULL,"
                                      "[%5] VARCHAR(250) NULL").arg(ProjectStorage::TABLE_NAME).arg(ProjectStorage::COL_CVS).arg(ProjectStorage::COL_CVS_URL)
                    .arg(ProjectStorage::COL_CVS_USERNAME).arg(ProjectStorage::COL_CVS_PASSWORD);
                this->db.exec(sql);

         }
     }


    DatabaseHelper* DatabaseHelper::getDatabase(QString filename)
    {
        if(!dbLists.contains(filename)){
            DatabaseHelper* db = new DatabaseHelper(filename);
            dbLists[filename] = db;
        }
        return dbLists[filename];
    }

    DatabaseHelper* DatabaseHelper::getDatabase()
    {
        return DatabaseHelper::getDatabase(QString(DBNAME));
    }

    DatabaseHelper* DatabaseHelper::getDatabase(long long id)
    {
        QString filename = QString(DBNAMEPRJ).arg(id);
        //qDebug()<<"filename:"<<filename;
        return DatabaseHelper::getDatabase(filename);
    }

    void DatabaseHelper::remove(long long id)
    {
        QString filename = QString(DBNAMEPRJ).arg(id);
        if(dbLists.contains(filename)){
            dbLists[filename]->close();
            dbLists.remove(filename);
        }
    }

}
