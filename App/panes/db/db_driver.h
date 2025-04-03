#ifndef DB_DRIVER_H
#define DB_DRIVER_H
#include "global.h"
#include <QString>
#include <QPair>
#include <QVariant>
#include <QList>
#include <QSqlQuery>
#include <QSqlField>
#include "table_field.h"
#include <tuple>

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
    virtual QList<QSqlField> tableFields(const QString name)=0;
    virtual std::tuple<QList<QSqlField>,QList<QList<QVariant>>,long long> queryData(const QString& table,const QString& where={},QList<QVariant>whereValues={},const QString& order={},int offset=0,int num=100)=0;


public:
    DBDriverPrivate* d;
};
}
#endif // DB_DRIVER_H
