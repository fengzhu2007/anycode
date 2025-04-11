#ifndef TABLE_FIELD_H
#define TABLE_FIELD_H
#include <QString>
namespace ady{
class TableField
{
public:
    QString name;
    QString type;
    int length=0;
    int decimal=0;
    QString defaultValue;
    bool primaryKey=false;
    bool uniqueValue=false;
    bool autoIncrement=false;
    bool notNull=false;

};
}
#endif // TABLE_FIELD_H
