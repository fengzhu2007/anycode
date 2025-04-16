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

    virtual QList<TableField> tableFields(const QString& name) override;
    virtual std::tuple<QList<QSqlField>,QList<QList<QVariant>>,long long> queryData(const QString& table,const QString& where={},QList<QVariant>whereValues={},const QString& order={},int offset=0,int num=100) override;

    virtual bool updateTableFields(const QString& name,const QList<TableField>& ofields ,const QList<TableField>& nfields) override;

    bool tableExists(const QString& name);

    bool addColumn(const QString &tableName, const TableField& field);
    bool editColumn(const QString &tableName, const TableField& ofield,const TableField& nfield);
    bool dropColumn(const QString &tableName, const TableField& field);





private:
    void parseFieldType(TableField& field,const QString& type);
    bool alterTableByRecreation(const QString &tableName,const QList<TableField>& ofields ,const QList<TableField>& nfields);

private:
    SQliteDriverPrivate* d;
};
}
#endif // SQLITE_DRIVER_H
