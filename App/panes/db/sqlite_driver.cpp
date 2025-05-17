#include "sqlite_driver.h"
#include "storage/db_storage.h"
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlDriver>
#include <QSqlIndex>
namespace ady{

class SQliteDriverPrivate{
public:
    QSqlDatabase db;
    QSqlError error;
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


QList<TableIndex> SQliteDriver::tableIndexes(const QString& name){
    QSqlQuery query(d->db);
    query.prepare(QString::fromUtf8("SELECT * FROM sqlite_master WHERE type = 'index' AND tbl_name =?"));
    query.bindValue(0,name);
    QList<TableIndex> list;
    while (query.next()) {

        QString sql = query.value("sql").toString();
        TableIndex index;
        this->parseIndex(index,sql);

        list.append(index);
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

void SQliteDriver::parseIndex(TableIndex& tIndex,const QString& sql){

    /*CREATE INDEX "type"
            ON "addon" (
              "typename" COLLATE NOCASE ASC,
              "typelabel"
            )*/
    QList<TableIndexColumn> columns;
    int step = 0;//0=find index name,1=find field,
    int index = 0;
    int start = 0;
    while(index < sql.length()){
        QChar ch = sql.at(index);
        switch(ch.unicode()){
        case '"':
        case '\'':
            if(start==0){
                start = index+1;//quote start
            }else{
                if(step==0){
                    //find index name
                    QString name = sql.mid(start,index - start);
                    tIndex.name = name;
                    step = 10;//set unknow
                    qDebug()<<"name"<<name;
                }else if(step==1){
                    //find column name
                    QString name = sql.mid(start,index - start);
                    step=2;//find field extra info
                    //columns.append()
                }
                start = 0;
            }
            break;
        case '(':
            step = 1;
            break;
        case ',':
            step = 1;//reset to find field
            break;


        }
        index++;
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
        auto fieldChanged = [](const TableField& field,const QList<TableField>& fields){
            for(auto one:fields){
                if(one.id==field.id){
                    return field.equal(one)?0:1;
                }
            }
            return -1;
        };

        QList<TableField> list;
        bool changed = false;

        for(auto one:nfields){
            auto ret = fieldChanged(one,ofields);
            if(ret==1){
                changed = true;
                break;
            }else if(ret==-1){
                list.append(one);
            }
        }
        for(auto one:ofields){
            auto ret = fieldChanged(one,nfields);
            if(ret==-1){
                changed = true;
                break;
            }
        }
        //qDebug()<<"changed"<<changed<<list.size();

        if(changed){
            return this->alterTableByRecreation(name,ofields,nfields);
        }else if(list.size()>0){
            return this->addColumns(name,list);
        }else{
            return true;
        }
    }else{
        //add table
        QString sql = QString::fromUtf8("CREATE TABLE [%1] (").arg(name);
        for(int i=0;i<nfields.length();i++){
            auto one = nfields.at(i);
            sql += QString::fromUtf8("[%1] %2").arg(one.name).arg(one.type);
            if(one.length>0){
                if(one.decimal>0){
                    sql += QString::fromUtf8("(%1,%2)").arg(one.length).arg(one.decimal);
                }else{
                    sql += QString::fromUtf8("(%1)").arg(one.length);
                }
            }
            if(one.primaryKey){
                sql += QString::fromUtf8(" PRIMARY KEY");
            }
            if(one.autoIncrement){
                sql += QString::fromUtf8(" AUTOINCREMENT");
            }
            if(!one.defaultValue.isEmpty()){
                sql += QString::fromUtf8(" DEFAULT %1").arg(one.defaultValue);
            }
            if(one.notNull){
                sql += QString::fromUtf8(" NOT NULL");
            }else{
                sql += QString::fromUtf8(" NULL");
            }
            if(i<nfields.length() - 1){
                sql += QString::fromUtf8(",");
            }
        }
        sql += QLatin1String(");");

        QSqlQuery query = d->db.exec(sql);
        d->error = query.lastError();
        return (d->error.type()==QSqlError::NoError);

    }

}

bool SQliteDriver::updateTableIndexes(const QString& name,const QList<TableIndex>& oIndexes ,const QList<TableIndex>& nIndexes){
    if(oIndexes.length()==0){
        //create index
        if(!d->db.transaction()){
            qDebug() << "Failed to start transaction:" << d->db.lastError().text();
            return false;
        }
        try{
            for(auto index:nIndexes){
                QString sql = QString::fromUtf8("CREATE INDEX [%1] ON [%2] (").arg(index.name).arg(name);
                int i=0;
                for(auto field:index.fields){
                    sql += QString::fromUtf8("[%1] %2  %3").arg(field.name).arg(field.collate.isEmpty()?"":("COLLATE "+ field.collate)).arg(field.order);
                    if(i<index.fields.length() - 1){
                        sql += ",";
                    }
                }
                sql += ")";
                QSqlQuery query = d->db.exec(sql);
                d->error = query.lastError();
                if(d->error.type()!=QSqlError::NoError){
                    throw QString("Failed to add index: %1").arg(index.name);
                }
            }
            d->db.commit();
            return true;
        }catch(const QString &error){
            d->db.rollback();
            qDebug()<<"error"<<error;
            return false;
        }

    }else{
        //update index
        auto indexChanged = [](const TableIndex& index,const QList<TableIndex>& indexes){
            int i = 0;
            for(auto one:indexes){
                if(one.id==index.id){
                    return index.equal(one)?-1:i;
                }
                i++;
            }
            //-1 index no change
            //>=0 index changed
            return -2;//new index
        };

        if(!d->db.transaction()){
            qDebug() << "Failed to start transaction:" << d->db.lastError().text();
            return false;
        }

        try{
            QList<TableIndex> list;
            for(auto one:nIndexes){
                auto ret = indexChanged(one,oIndexes);
                if(ret>=0){
                    //drop before index
                    //create new index
                    auto name = oIndexes.at(ret).name;
                    QString sql = QString::fromUtf8("DROP INDEX IF EXISTS [%1]").arg(name);
                    QSqlQuery query = d->db.exec(sql);
                    d->error = query.lastError();
                    if(d->error.type()!=QSqlError::NoError){
                        throw QString("Failed to drop index: %1").arg(name);
                    }
                    list.append(one);
                    break;
                }else if(ret==-2){
                    //new index
                    list.append(one);
                }
            }

            for(auto one:oIndexes){
                auto ret = indexChanged(one,nIndexes);
                if(ret==-2){
                    //drop old index
                    QString sql = QString::fromUtf8("DROP INDEX IF EXISTS [%1]").arg(one.name);
                    QSqlQuery query = d->db.exec(sql);
                    d->error = query.lastError();
                    if(d->error.type()!=QSqlError::NoError){
                        throw QString("Failed to drop index: %1").arg(one.name);
                    }
                }
            }


            for(auto one:list){
                QString sql = QString::fromUtf8("CREATE INDEX [%1] ON [%2] (").arg(one.name).arg(name);
                int i=0;
                for(auto field:one.fields){
                    sql += QString::fromUtf8("[%1] %2  %3").arg(field.name).arg(field.collate.isEmpty()?"":("COLLATE "+ field.collate)).arg(field.order);
                    if(i<one.fields.length() - 1){
                        sql += ",";
                    }
                }
                sql += ")";
                QSqlQuery query = d->db.exec(sql);
                d->error = query.lastError();
                if(d->error.type()!=QSqlError::NoError){
                    throw QString("Failed to add index: %1").arg(one.name);
                }
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

QList<QPair<QString,int>> SQliteDriver::fieldTypes(){
    QList<QPair<QString,int>> list;
    list.append({"INTEGER",0});
    list.append({"FLOAT",0});
    list.append({"REAL",0});
    list.append({"NUMERIC",0});
    list.append({"BOOLEAN",0});
    list.append({"DECIMAL",2});
    list.append({"DATE",0});
    list.append({"DATETIME",0});
    list.append({"TIME",0});
    list.append({"VARCHAR",1});
    list.append({"NVARCHAR",1});
    list.append({"TEXT",0});
    list.append({"BLOB",0});
    return list;
}


QSqlError SQliteDriver::lastError(){
    return d->error;
}

bool SQliteDriver::insert(const QString& name,const QList<QSqlField>& fields,QList<QVariant>& data){
    auto sql = QString::fromUtf8("INSERT INTO [%1] (").arg(name);
    QStringList list;
    QStringList placeholderlist;
    QList<QVariant> valuelist;
    for(int i=0;i<fields.length();i++){
        auto field = fields.at(i);
        auto val = data.at(i);
        if(val.isValid()){
            list.append(QString::fromUtf8("[%1]").arg(field.name()));
            placeholderlist.append("?");
            valuelist.append(val);
        }
    }
    if(valuelist.size()==0){
        return false;
    }
    sql += list.join(",")+") VALUES ("+placeholderlist.join(",")+")";
    qDebug()<<"sql"<<sql;
    QSqlQuery query(d->db);
    query.prepare(sql);
    int i = 0;
    for(auto one:valuelist){
        query.bindValue(i++,one);
    }
    auto ret = query.exec();
    if(!ret){
        d->error = query.lastError();
    }else{
        QSqlQuery query(d->db);
        QStringList selectlist;
        for(auto one:fields){
            selectlist.append(QString::fromUtf8("[%1]").arg(one.name()));
        }
        query.exec(QString::fromUtf8("SELECT %1 FROM [%2] WHERE rowid = last_insert_rowid()").arg(selectlist.join(",")).arg(name));
        if(query.next()) {
            for(int i=0;i<fields.length();i++){
                data[i] = query.value(i);
            }
        }
    }
    return ret;
}

bool SQliteDriver::update(const QString& name,const QList<QSqlField>& fields,const QList<QVariant>& oData,const QList<QVariant>& nData){

        //update
        QSqlIndex primaryIndex = d->db.primaryIndex(name);
        QString sql = QString::fromUtf8("UPDATE [%1] SET ").arg(name);
        QStringList placeholderlist;
        QList<QVariant> valuelist;
        QStringList whereList;
        QList<QVariant> whereValue;
        for(int i=0;i<fields.length();i++){
            auto field = fields.at(i);
            auto oOne = oData.at(i);
            auto oNew = nData.at(i);
            if(oOne!=oNew){
                placeholderlist.append(QString::fromUtf8("[%1]=?").arg(field.name()));
                valuelist.append(oNew);
            }
            if(primaryIndex.contains(field.name())){
                whereList.append(QString::fromUtf8("[%1]=?").arg(field.name()));
                whereValue.append(oOne);
            }
        }
        sql += placeholderlist.join(",") + QString::fromUtf8(" WHERE ") + whereList.join(" AND ");
        QSqlQuery query(d->db);
        query.prepare(sql);
        int i = 0;
        for(auto one:valuelist){
            query.bindValue(i++,one);
        }
        for(auto one:whereValue){
            query.bindValue(i++,one);
        }
        auto ret = query.exec();
        if(!ret){
            d->error = query.lastError();
        }
        return ret;

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
    auto sql = QString::fromUtf8("ALTER TABLE [%1] ADD COLUMN [%2] %3").arg(tableName).arg(field.name).arg(field.type);
    if(field.length>0){
        if(field.decimal>0){
            sql += QString::fromUtf8("(%1,%2)").arg(field.length).arg(field.decimal);
        }else{
            sql += QString::fromUtf8("(%1)").arg(field.length);
        }
    }
    if(field.primaryKey){
        sql += QString::fromUtf8(" PRIMARY KEY");
    }
    if(field.autoIncrement){
        sql += QString::fromUtf8(" AUTOINCREMENT");
    }
    if(!field.defaultValue.isEmpty()){
        sql += QString::fromUtf8(" DEFAULT %1").arg(field.defaultValue);
    }
    if(field.notNull){
        sql += QString::fromUtf8(" NOT NULL");
    }else{
        sql += QString::fromUtf8(" NULL");
    }
    QSqlQuery query = d->db.exec(sql);
    d->error = query.lastError();
    return (d->error.type()==QSqlError::NoError);
}

bool SQliteDriver::addColumns(const QString &tableName, const QList<TableField> fields){
    if(!d->db.transaction()){
        qDebug() << "Failed to start transaction:" << d->db.lastError().text();
        return false;
    }
    try{
        for(auto one:fields){
            auto ret = this->addColumn(tableName,one);
            if(!ret){
                throw QString("Failed to add column: %1").arg(one.name);
            }
        }
        d->db.commit();
        return true;
    }catch(const QString &error){
        d->db.rollback();
        qDebug()<<"error"<<error;
        return false;
    }
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
        if(!ret){
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
            d->error = query.lastError();
            throw QString("Failed to copy data: %1").arg(query.lastError().text());
        }

        if (!query.exec(QString("DROP TABLE [%1]").arg(tableName))) {
            d->error = query.lastError();
            throw QString("Failed to drop original table: %1").arg(query.lastError().text());
        }
        if (!query.exec(QString("ALTER TABLE [%1] RENAME TO [%2]").arg(name, tableName))) {
            d->error = query.lastError();
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
