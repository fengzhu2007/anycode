#ifndef TABLE_FIELD_H
#define TABLE_FIELD_H
#include <QString>
namespace ady{
class TableField
{
public:
    TableField():id(TableField::seq++){

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
    inline bool equal(const TableField& other){
        if(name==other.name && type==other.type && length==other.length && decimal==other.decimal && primaryKey==other.primaryKey && autoIncrement==other.autoIncrement
            && notNull==other.notNull){
            return true;
        }else{
            return false;
        }
    }

public:
    static long long seq;

};
}
#endif // TABLE_FIELD_H
