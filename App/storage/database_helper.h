#ifndef DATABASEHELPER_H
#define DATABASEHELPER_H
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMap>
#include "global.h"


namespace ady {

    class ANYENGINE_EXPORT DatabaseHelper{
    public:

        const QString VERSION           =   "version";

        constexpr const static char COL_ID[]  =   "id";
        constexpr const static char COL_DATETIME[]   =   "datetime";

        constexpr const static char DBNAME[] = "data.s3db";
        constexpr const static char DBNAMEPRJ[] = "project%1.s3db";//project database

        const static int VERSIONID = 7;


    private:
        DatabaseHelper(QString filename);
        bool open();
        bool open(QString username,QString password);
        int dbType();
        void close();



    public:
        ~DatabaseHelper();
        static DatabaseHelper* getDatabase(QString filename);
        static DatabaseHelper* getDatabase();
        static DatabaseHelper* getDatabase(long long id);
        static void remove(long long id);
        bool tableExists(QString tablename);
        int getVersion();
        bool setVersion();
        QSqlDatabase get();



    private:
        void onUpgrade();


        void upgradeV0();
        void upgradeV1();
        void upgradeV2();
        void upgradeV3();
        void upgradeV4();
        void upgradeV5();
        void upgradeV6();


    public:
        static DatabaseHelper* instance;
        static QMap<QString,DatabaseHelper*> dbLists;

    private:
        QString dbname;
        QSqlDatabase db;



    };
}
#endif // DATABASEHELPER_H
