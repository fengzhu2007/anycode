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
    virtual QList<TableIndex> tableIndexes(const QString& name) override;
    virtual std::tuple<QList<QSqlField>,QList<QList<QVariant>>,long long> queryData(const QString& table,const QString& where={},QList<QVariant>whereValues={},const QString& order={},int offset=0,int num=100) override;

    virtual bool updateTableFields(const QString& name,const QList<TableField>& ofields ,const QList<TableField>& nfields) override;
    virtual bool updateTableIndexes(const QString& name,const QList<TableIndex>& oIndexes ,const QList<TableIndex>& nIndexes) override;

    virtual QList<QPair<QString,int>>fieldTypes() override;
    virtual QSqlError lastError() override;
    virtual bool insert(const QString& name,const QList<QSqlField>& fields,QList<QVariant>& data) override;
    virtual bool update(const QString& name,const QList<QSqlField>& fields,const QList<QVariant>& oData,const QList<QVariant>& nData) override;
    virtual bool del(const QString& name,const QList<QSqlField>& fields,QList<QVariant>& data ) override;
    virtual bool rename(const QString& oldName,const QString& newName) override;
    virtual bool dropTable(const QString& name) override;
    virtual QSqlError error() override;
    virtual QString errorText() override;


    bool tableExists(const QString& name);

    bool addColumn(const QString &tableName, const TableField& field);
    bool addColumns(const QString &tableName, const QList<TableField> fields);



    bool editColumn(const QString &tableName, const TableField& ofield,const TableField& nfield);
    bool dropColumn(const QString &tableName, const TableField& field);





private:
    void parseFieldType(TableField& field,const QString& type);
    void parseIndex(TableIndex& index,const QString& sql);
    bool alterTableByRecreation(const QString &tableName,const QList<TableField>& ofields ,const QList<TableField>& nfields);

private:
    SQliteDriverPrivate* d;
};
}
#endif // SQLITE_DRIVER_H
