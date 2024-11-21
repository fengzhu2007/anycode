#ifndef COMMONSTORAGE_H
#define COMMONSTORAGE_H
#include <QString>
#include "storage.h"
#include "global.h"
#include <QJsonObject>

namespace ady {


class ANYENGINE_EXPORT CommonRecord{
public:
    QString name;
    QString value;
    QString group;
};





    class ANYENGINE_EXPORT CommonStorage : public Storage{
    public:
        constexpr const static char  TABLE_NAME[] = "common";
        constexpr const static char  COL_NAME[]   = "name";
        constexpr const static char  COL_VALUE[]  = "value";
        constexpr const static char  COL_GROUP[]  = "group";

        static const QString VERSION;
        static const QString OPTIONS;


        CommonStorage();
        CommonRecord one(const QString& name);
        bool update(const CommonRecord& record);
        bool insert(const CommonRecord& record);
        bool replace(const CommonRecord& record);
        bool replace(const QString& name,const QString& value);

        bool del(const QString& name);
        QJsonObject options();
    private:
        CommonRecord toRecord(QSqlQuery& query);


    };
}
#endif // COMMONSTORAGE_H
