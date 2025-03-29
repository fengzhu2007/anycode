#include "sqlite_driver.h"
#include "storage/db_storage.h"
#include <QSqlDatabase>
namespace ady{

class SQliteDriverPrivate{
public:
    QSqlDatabase db;
};

SQliteDriver::SQliteDriver(const DBRecord& data):DBDriver(data) {

    d = new SQliteDriverPrivate;
    d->db = QSqlDatabase::addDatabase("QSQLITE"/*,data.host*/);
    qDebug()<<"file"<<data.host;
    //d->db.setDatabaseName("main");
    d->db.setDatabaseName(data.host);

}

SQliteDriver::~SQliteDriver(){
    d->db.close();
    delete d;
}

bool SQliteDriver::connect(){
    return d->db.open();
}

QStringList SQliteDriver::dbList(){
    //QStringList list = {d->db.databaseName()};
    QStringList list = {"main"};
    return list;
}

QStringList SQliteDriver::tableList(){
    QSqlQuery query(d->db);
    if (query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name ASC")) {
        QStringList tablelist;
        while (query.next()) {
            tablelist.append(query.value(0).toString());
        }
        return tablelist;
    } else {
        return {};
    }
}

QStringList SQliteDriver::viewList(){
    return d->db.tables(QSql::Views);
}


}
