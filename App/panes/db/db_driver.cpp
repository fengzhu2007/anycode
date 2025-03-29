#include "db_driver.h"
#include "storage/db_storage.h"
#include "dbms_model.h"
namespace ady{

class DBDriverPrivate{
public:
    explicit DBDriverPrivate(const DBRecord& record):data(record){

    }

    DBRecord data;
};

DBDriver::DBDriver(const DBRecord& data) {
    d = new DBDriverPrivate(data);
}

DBDriver::~DBDriver(){
    delete d;
}

QString& DBDriver::name() const{
    return d->data.driver;
}

DBRecord& DBDriver::data() const{
    return d->data;
}

QList<QPair<int,QString>> DBDriver::typeList(){
    QList<QPair<int,QString>> list;
    list.append({DBDriver::Table,QObject::tr("Table")});
    list.append({DBDriver::View,QObject::tr("View")});
    return list;
}

}



