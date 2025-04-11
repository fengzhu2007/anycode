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

QList<TableField> SQliteDriver::tableFields(const QString name) {

    QSqlQuery query(d->db);
    QString sql = QString::fromUtf8("PRAGMA table_info([%1])").arg(name);
    query.prepare(sql);
    if (!query.exec()) {
        qDebug() << "Error getting table info:" << query.lastError().text();
        return {};
    }
    QList<TableField> list;
    while (query.next()) {
        TableField field;
        field.name = query.value("name").toString();
        QString type = query.value("type").toString();
        this->parseFieldType(field,type);
        field.notNull = query.value("notnull").toInt()>0;
        field.defaultValue = query.value("dflt_value").toString();
        field.primaryKey = query.value("pk").toInt()>0;
        if(field.primaryKey && type.toUpper()==QLatin1String("INTEGER")){
            //search sqlite_sequence
            QSqlQuery seqQuery("SELECT [name] FROM sqlite_sequence WHERE name='" + name + "'");
            if(seqQuery.exec() && seqQuery.next()){
                field.autoIncrement = true;
            }
        }
        list.append(field);
    }
    return list;
}



void SQliteDriver::parseFieldType(TableField& field,const QString& type){
    int index = 0;
    int start = 0;
    int step = 0;//0=name,1=length,2=decimal
    while(index < type.length()){
        QChar ch = type.at(index);
        switch(ch.unicode()){
        case '(':
            if(step==0){
                field.type = type.mid(start,index - start);
            }
            start = index+1;
            step = 1;
            break;
        case ')':
            if(step==1){
                 auto length = type.mid(start,index - start);
                 field.length = length.toInt();

            }else if(step==2){
                //decimal
                auto decimal = type.mid(start,index - start);
                field.decimal = decimal.toInt();
            }
            break;
        case ',':
            if(step==1){
                //save length
                auto length = type.mid(start,index - start);
                field.length = length.toInt();
            }
            start = index+1;
            step = 2;
            break;
        }
        index++;
    }
    if(step==0){
        field.type = type;
    }

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
