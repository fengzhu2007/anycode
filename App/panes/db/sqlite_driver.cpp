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

QList<TableField> SQliteDriver::tableFields(const QString& name) {

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

bool SQliteDriver::updateTableFields(const QString& name,const QList<TableField>& ofields ,const QList<TableField>& nfields){

    if(tableExists(name)){
        //update
        QList<TableField> list;
        bool changed = false;
        for(auto nOne:nfields){
            bool exists = false;
            for(auto oOne:ofields){
                if(nOne.id==oOne.id){
                    exists = true;
                    if(!oOne.equal(nOne)){
                        changed = true;
                    }
                }
            }
            if(exists==false){
                //new
                list.append(nOne);
            }
        }

        if(changed || ofields.length()!=nfields.length()){
            this->alterTableByRecreation(name,ofields,nfields);
        }else if(list.length()>0){
            for(auto one:list){
                this->addColumn(name,one);
            }
        }
        return false;


    }else{
        //add table
        QString sql = QString::fromUtf8("CREATE TABLE [%1] (").arg(name);
        for(int i=0;i<nfields.length();i++){
            auto one = nfields.at(i);
            sql += QString::fromUtf8("[%1] ").arg(one.name);
            sql += one.type;
            if(one.length>0){
                if(one.decimal>0){
                    sql += QLatin1String("(%1,%2)").arg(one.length).arg(one.decimal);
                }else{
                    sql += QLatin1String("(%1)").arg(one.length);
                }
            }
            if(one.primaryKey){
                sql += QLatin1String(" PRIMARY KEY");
            }
            if(one.autoIncrement){
                sql += QLatin1String(" AUTOINCREMENT");
            }
            if(!one.defaultValue.isEmpty()){
                sql += QLatin1String(" DEFAULT %1").arg(one.defaultValue);
            }
            if(one.notNull){
                sql += QLatin1String(" NOT NULL");
            }else{
                sql += QLatin1String(" NULL");
            }
            if(i<nfields.length() - 1){
                sql += QLatin1String(",");
            }
        }
        sql += QLatin1String(");");

        QSqlQuery query(d->db);
        query.prepare(sql);
        auto ret = query.exec();
        return ret;

    }

}

bool SQliteDriver::tableExists(const QString& name){
    QSqlQuery query(d->db);
    query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name=:name");
    query.bindValue(":name", name);
    if (!query.exec()) {
        qWarning() << "Query failed:" << query.lastError();
        return false;
    }
    return query.next();
}


bool SQliteDriver::addColumn(const QString &tableName, const TableField& field){
    auto sql = QString::fromUtf8("ALTER TABLE [%1] ADD COLUMN [%2]").arg(tableName).arg(field.name);
    if(field.length>0){
        if(field.decimal>0){
            sql += QLatin1String("(%1,%2)").arg(field.length).arg(field.decimal);
        }else{
            sql += QLatin1String("(%1)").arg(field.length);
        }
    }
    if(field.primaryKey){
        sql += QLatin1String(" PRIMARY KEY");
    }
    if(field.autoIncrement){
        sql += QLatin1String(" AUTOINCREMENT");
    }
    if(!field.defaultValue.isEmpty()){
        sql += QLatin1String(" DEFAULT %1").arg(field.defaultValue);
    }
    if(field.notNull){
        sql += QLatin1String(" NOT NULL");
    }else{
        sql += QLatin1String(" NULL");
    }
    QSqlQuery query(d->db);
    query.prepare(sql);
    return query.exec();
}

bool SQliteDriver::editColumn(const QString &tableName, const TableField& ofield,const TableField& nfield){
    return false;
}

bool SQliteDriver::dropColumn(const QString &tableName, const TableField& field){
    return false;
}

bool SQliteDriver::alterTableByRecreation(const QString &tableName,const QList<TableField>& ofields ,const QList<TableField>& nfields){

    auto existInOldFields = [](const TableField& field,const QList<TableField>& fields){
        for(auto one:fields){
            if(one.id==field.id){
                return one.name;
            }
        }
        return QString();
    };



    if(!d->db.transaction()){
        qDebug() << "Failed to start transaction:" << d->db.lastError().text();
        return false;
    }

    try{
        QString name = QString::fromUtf8("%1_temp").arg(tableName);
        auto ret = this->updateTableFields(name,{},nfields);
        if(ret){
            throw QString::fromUtf8("Create temp table failed");
        }
        QStringList oList;
        QStringList nList;

        for(auto nOne:nfields){
            auto name = existInOldFields(nOne,ofields);
            if(!name.isEmpty()){
                oList.append(QString::fromUtf8("[%1]").arg(name));
                nList.append(QString::fromUtf8("[%1]").arg(nOne.name));
            }
        }
        QSqlQuery query(d->db);
        QString sql = QString::fromUtf8("INSERT INTO [%1] (%2) SELECT %3 FROM [%4] ").arg(name).arg(nList.join(",")).arg(oList.join(",")).arg(tableName);
        if (!query.exec(sql)) {
            throw QString("Failed to copy data: %1").arg(query.lastError().text());
        }

        if (!query.exec(QString("DROP TABLE [%1]").arg(tableName))) {
            throw QString("Failed to drop original table: %1").arg(query.lastError().text());
        }
        if (!query.exec(QString("ALTER TABLE [%1] RENAME TO [%2]").arg(name, tableName))) {
            throw QString("Failed to rename temp table: %1").arg(query.lastError().text());
        }
        d->db.commit();
        return true;
    }catch(const QString &error){
        d->db.rollback();
        qDebug()<<"error"<<error;
        return false;
    }
}

}
