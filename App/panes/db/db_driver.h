#ifndef DB_DRIVER_H
#define DB_DRIVER_H
#include "global.h"
#include <QString>
#include <QPair>
namespace ady{

class DBRecord;
class DBDriverPrivate;
class ANYENGINE_EXPORT DBDriver
{
public:
    enum Type{
        Table=0,
        View,
    };
    DBDriver(const DBRecord& data);
    virtual ~DBDriver();
    QString& name() const;//driver name
    DBRecord& data() const;

    virtual bool connect()=0;//connect database link
    virtual QStringList dbList() = 0;//database list
    virtual QList<QPair<int,QString>> typeList();
    virtual QStringList tableList()=0;
    virtual QStringList viewList()=0;

public:
    DBDriverPrivate* d;
};
}
#endif // DB_DRIVER_H
