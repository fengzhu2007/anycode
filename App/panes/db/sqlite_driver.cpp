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
    d->db = QSqlDatabase::addDatabase("QSQLITE",data.host);
    d->db.setDatabaseName("main");

}

SQliteDriver::~SQliteDriver(){
    d->db.close();
    delete d;
}

bool SQliteDriver::connect(){
    return d->db.open();
}

QStringList SQliteDriver::dbList(){
    QStringList list = {d->db.databaseName()};
    return list;
}





}
