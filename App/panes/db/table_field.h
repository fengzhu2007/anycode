#ifndef TABLE_FIELD_H
#define TABLE_FIELD_H
#include <QString>
namespace ady{
class TableField
{
public:
    QString name;
    QString type;
    int length;
    int decimal;
    QString defaultValue;
    bool primaryKey;
    bool uniqueValue;
    bool autoIncrement;
    bool notNull;

};
}
#endif // TABLE_FIELD_H
