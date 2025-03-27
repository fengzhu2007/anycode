#include "db_driver.h"
#include "storage/db_storage.h"
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


}



