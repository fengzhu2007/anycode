#ifndef DB_DRIVER_H
#define DB_DRIVER_H
#include "global.h"
#include <QString>
namespace ady{

class DBRecord;
class DBDriverPrivate;
class ANYENGINE_EXPORT DBDriver
{
public:
    DBDriver(const DBRecord& data);
    virtual ~DBDriver();
    QString& name() const;//driver name
    DBRecord& data() const;

    virtual bool connect()=0;//connect database link
    virtual QStringList dbList() = 0;//database list

public:
    DBDriverPrivate* d;
};
}
#endif // DB_DRIVER_H
