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

struct ANYENGINE_EXPORT OpenFindData{
    int mode;
    QString text;
    QString scope;
};

struct ANYENGINE_EXPORT OpenEditorData{
    QString path;
    int line=0;
    int column=0;
};

}
#endif // EVENT_DATA_H
