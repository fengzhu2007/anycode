#include "sqlite_driver.h"
#include "storage/db_storage.h"
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlDriver>
namespace ady{

class SQliteDriverPrivate{
public:
    QSqlDatabase db;
};

SQliteDriver::SQliteDriver(const DBRecord& data):DBDriver(data) {

    d = new SQliteDriverPrivate;
    d->db = QSqlDatabase::addDatabase("QSQLITE"/*,data.host*/);
    //qDebug()<<"file"<<data.host;
    //d->db.setDatabaseName("main");
    d->db.setDatabaseName(data.host);

}

SQliteDriver::~SQliteDriver(){
    d->db.close();
    delete d;
}

bool SQliteDriver::connect(){
    return d->db.open();
}

QStringList SQliteDriver::dbList(){
    //QStringList list = {d->db.databaseName()};
    QStringList list = {"main"};
    return list;
}

QStringList SQliteDriver::tableList(){
    QSqlQuery query(d->db);
    if (query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name ASC")) {
        QStringList tablelist;
        while (query.next()) {
            tablelist.append(query.value(0).toString());
        }
        return tablelist;
    } else {
        return {};
    }
}

QStringList SQliteDriver::viewList(){
    return d->db.tables(QSql::Views);
}

QList<QSqlField> SQliteDriver::tableFields(const QString name) {
    QSqlRecord record = d->db.record(name);
    QList<QSqlField> fields;
    auto count = record.count();
    for (int i = 0; i < count; ++i) {
        fields.append(record.field(i));
    }
    return fields;
}

std::tuple<QList<QSqlField>,QList<QList<QVariant>>,long long> SQliteDriver::queryData(const QString& table,const QString& where,QList<QVariant>whereValues,const QString& order,int offset,int num){
    QSqlQuery query(d->db);
    QString sql = QString::fromUtf8("SELECT * FROM [%1] WHERE 1=1 %2 %3 LIMIT %4,%5").arg(table).arg(where.isEmpty()?where:(" AND "+where)).arg(order.isEmpty()?order:(" ORDER BY "+order)).arg(offset).arg(num);
    query.prepare(sql);
    int i = 0;
    for(auto one:whereValues){
        query.bindValue(i++,one);
    }
    auto ret = query.exec();
    if(ret){
        int total = 0;
        QSqlRecord record = query.record();
        auto count = record.count();
        QList<QSqlField> fields;
        for (int i = 0; i < count; ++i) {
            fields.append(record.field(i));
        }
        QList<QList<QVariant>> list;
        while(query.next()){
            QList<QVariant> item;
            for(int i=0;i<count;i++){
               item.append(query.value(i));
            }
            list.append(item);
        }
        //d->db.driver()->FieldName();
        return std::make_tuple(std::move(fields),std::move(list),total);
    }else{
        return std::make_tuple(QList<QSqlField>{},QList<QList<QVariant>>{},0);
    }
}


}
