#ifndef SQLITE_DRIVER_H
#define SQLITE_DRIVER_H

#include "global.h"
#include "db_driver.h"
namespace ady{
class DBRecord;
class SQliteDriverPrivate;
class ANYENGINE_EXPORT SQliteDriver : public DBDriver
{
public:
    SQliteDriver(const DBRecord& data);
    virtual ~SQliteDriver() override;

    virtual bool connect() override;
    virtual QStringList dbList() override;


private:
    SQliteDriverPrivate* d;
};
}
#endif // SQLITE_DRIVER_H
