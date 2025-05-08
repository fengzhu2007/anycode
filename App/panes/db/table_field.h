#ifndef TABLE_FIELD_H
#define TABLE_FIELD_H
#include <QString>
#include <QList>
namespace ady{
class TableField
{
public:
    TableField():id(TableField::seq++),length(0),decimal(0),primaryKey(false),autoIncrement(false),notNull(false){

    }

    TableField(const TableField& other){
        this->id = other.id;
        this->name = other.name;
        this->type = other.type;
        this->length = other.length;
        this->decimal = other.decimal;
        this->defaultValue = other.defaultValue;
        this->primaryKey = other.primaryKey;
        this->autoIncrement = other.autoIncrement;
        this->notNull = other.notNull;
    }

    TableField& operator=(const TableField& other) {
        this->id = other.id;
        this->name = other.name;
        this->type = other.type;
        this->length = other.length;
        this->decimal = other.decimal;
        this->defaultValue = other.defaultValue;
        this->primaryKey = other.primaryKey;
        this->autoIncrement = other.autoIncrement;
        this->notNull = other.notNull;
        return *this;
    }


    long long id;
    QString name;
    QString type;
    int length=0;
    int decimal=0;
    QString defaultValue;
    bool primaryKey=false;
    //bool uniqueValue=false;
    bool autoIncrement=false;
    bool notNull=false;
    inline bool equal(const TableField& other) const{
        if(name==other.name && type==other.type && length==other.length && decimal==other.decimal && primaryKey==other.primaryKey && autoIncrement==other.autoIncrement
            && notNull==other.notNull && defaultValue==other.defaultValue){
            return true;
        }else{
            return false;
        }
    }

public:
    static long long seq;

};


class TableIndexColumn{
public:
    QString name;//fieldname
    QString collate;
    QString order;
    inline bool equal(const TableIndexColumn& other){
        if(name==other.name && collate==other.collate && order==other.order){
            return true;
        }else{
            return false;
        }
    }
};

class TableIndex
{
public:
    TableIndex():id(TableIndex::seq++){

    }

    TableIndex(const TableIndex& other){
        this->id = other.id;
        this->name = other.name;
        this->isUnique = other.isUnique;
        this->fields = other.fields;

    }

    TableIndex& operator=(const TableIndex& other) {
        this->id = other.id;
        this->name = other.name;
        this->isUnique = other.isUnique;
        this->fields = other.fields;
        return *this;
    }

    bool equal(const TableIndex& other) const{
        if(name==other.name && isUnique==other.isUnique && fields.length()==other.fields.length()){
            for(auto one:fields){
                bool equal = false;
                for(auto ot:other.fields){
                    equal = one.equal(ot);
                    if(equal){
                        break;
                    }
                }
                if(!equal){
                    return false;
                }
            }
            return true;
        }else{
            return false;
        }
    }

public:

    long long id;
    QString name;
    bool isUnique=false;
    QList<TableIndexColumn> fields;

public:
    static long long seq;
};


}
#endif // TABLE_FIELD_H
