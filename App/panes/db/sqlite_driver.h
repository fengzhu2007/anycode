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
    virtual QStringList tableList() override;
    virtual QStringList viewList() override;

    virtual QList<QSqlField> tableFields(const QString name) override;
    virtual std::tuple<QList<QSqlField>,QList<QList<QVariant>>,long long> queryData(const QString& table,const QString& where={},QList<QVariant>whereValues={},const QString& order={},int offset=0,int num=100) override;


private:
    SQliteDriverPrivate* d;
};
}
#endif // SQLITE_DRIVER_H
