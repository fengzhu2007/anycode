#include "Storage.h"
#include <QSqlQuery>
#include <QSqlError>
namespace ady {
    Storage::Storage()
    {

    }

    QString Storage::errorText()
    {
        return this->error.text();
    }

    QString Storage::errorCode()
    {
        return this->error.nativeErrorCode();
    }
}
