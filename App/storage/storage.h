#ifndef STORAGE_H
#define STORAGE_H
#include <QString>
#include <QSqlQuery>
#include <QSqlError>
#include "global.h"
namespace ady {


    class ANYENGINE_EXPORT Storage{
    public:
        Storage();
        QString errorText();
        QString errorCode();


    protected:
        QSqlError error;


    };
}
#endif // STORAGE_H
