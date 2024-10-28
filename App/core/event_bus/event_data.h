#ifndef EVENT_DATA_H
#define EVENT_DATA_H

#include "global.h"
#include <QString>
#include <QList>
#include <QMap>
#include <QPair>
#include <QJsonObject>
namespace ady{

struct ANYENGINE_EXPORT UploadData{
    long long pid;
    long long siteid;
    bool is_file;
    QString source;
    QString dest;

    QJsonObject toJson(){
        return {
            {"pid",pid},
            {"siteid",siteid},
            {"is_file",is_file},
                {"source",source},
                {"dest",dest},
            };
    }
};

struct ANYENGINE_EXPORT DownloadData{
    long long pid;
    long long siteid;
    long long filesize;
    bool is_file;
    QString remote;
    QString local;

    QJsonObject toJson(){
        return {
                {"pid",pid},
                {"siteid",siteid},
                {"filesize",filesize},
                {"is_file",is_file},
                {"remote",remote},
                {"local",local},
                };
    }
};

struct ANYENGINE_EXPORT OpenFindData{
    int mode;
    QString text;
    QString scope;
    QJsonObject toJson(){
        return {
                {"mode",mode},
                {"text",text},
                {"scope",scope}
                };
    }
};

struct ANYENGINE_EXPORT OpenEditorData{
    QString path;
    int line=0;
    int column=0;
    QJsonObject toJson(){
        return {
            {"path",path},
            {"line",line},
            {"column",column}
        };
    }
};

struct ANYENGINE_EXPORT CloseProjectData{
    long long id;
    QString path;
    QJsonObject toJson(){
        return {
            {"id",id},
            {"path",path}
        };
    }
};

}
#endif // EVENT_DATA_H
