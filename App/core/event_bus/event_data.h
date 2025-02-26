#ifndef EVENT_DATA_H
#define EVENT_DATA_H

#include "global.h"
#include "interface/file_item.h"

#include <QString>
#include <QList>
#include <QMap>
#include <QPair>
#include <QJsonObject>
#include <QJsonArray>

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

struct ANYENGINE_EXPORT ServerRefreshData{
    int cmd;
    long long id;
    QString path;
    QList<FileItem> list;
    QJsonObject toJson(){
        QJsonArray array;
        for(auto one:list){
            array << one.toJson();
        }
        return {
            {"cmd",cmd},
            {"id",id},
            {"path",path},
            {"list",array},
        };
    }
};

struct ANYENGINE_EXPORT OutputData{
    int level;
    QString source;
    QString content;
    QJsonObject toJson(){
        return {
                {"level",level},
                {"source",source},
                {"content",content},
        };
    }
    void fromJson(const QJsonObject& data){
        this->level = data.find("level")->toInt(0);
        this->source = data.find("source")->toString();
        this->content = data.find("content")->toString();
    }
};


struct ANYENGINE_EXPORT NotificationData{
    QString type;
    QString title;
    QString description;
    QString time;
    QJsonObject data;
    QJsonObject toJson(){
        return {
            {"type",type},
            {"title",title},
            {"description",description},
            {"time",time},
            {"data",data},
        };
    }

    void fromJson(const QJsonObject& data){
        this->type = data.find("type")->toString();
        this->title = data.find("title")->toString();
        this->description = data.find("description")->toString();
        this->time = data.find("time")->toString();
        this->data = data.find("data")->toObject();
    }

};

}
#endif // EVENT_DATA_H
