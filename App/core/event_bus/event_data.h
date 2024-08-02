#ifndef EVENT_DATA_H
#define EVENT_DATA_H

#include "global.h"
#include <QString>
#include <QList>
#include <QPair>
namespace ady{

struct ANYENGINE_EXPORT UploadData{
    long long pid;
    bool is_file;
    QString source;
    QList<QPair<long long,QString>> dest;
};


}
#endif // EVENT_DATA_H
