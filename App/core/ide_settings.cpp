#include "ide_settings.h"
#include "idewindow.h"
#include <QJsonDocument>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
/*****************************DEMO*********************************/
/*

{
version:1,
width:800,
height:600,
maximized:0
dockpanes:{
    inner:{
            orientation:0,1,2,
            list:[
                {
                    client:0,
                    active:0,
                    tabs:[
                        {id:xxx,group:xxx,position:1,data:{}},
                    ]
                },
                {children:[
                        {
                            client:0,
                            active:0,
                            tabs:[
                                {id:xxx,group:xxx,position:1,data:{}},
                                {id:xxx,group:xxx,position:1,data:{}},
                            ]

                        },
                        {
                            client:1,
                            active:1,
                            tabs:[
                                {id:xxx,group:xxx,position:1,data:{}},
                                {id:xxx,group:xxx,position:1,data:{}},
                            ]

                        },
                    ]
                },
                {id:xxx,group:xxx,client:1,data:{}},
            ]
        },
    float:[
        {
            left:xx,top:xxx,width:xxx,height:xxx,
            active:0,
            tabs:[
                {id:xxx,group:xxx,position:1,data:{}},
                {id:xxx,group:xxx,position:1,data:{}},
            ]
        },
    ],
    fixed:[
        {id:xxx,group:xxx,position:1,data:{}},
    ]
}
projects:[
    {
        id:1,
        path:"xxxxxxx dir path",
        opened:[
            "xxxxx open folder path",
            "xxxxx open folder path"
        ]
    }
],


}


*/
/*****************************DEMO END*******************************/

namespace ady{


IDESettings* IDESettings::instance = nullptr;


class IDESettingsPrivate{
public:
    int version=1;
    int width=800;
    int height = 600;
    int maximized = 0;
    QJsonObject dockpanes;
    QJsonArray projects;
    IDEWindow* window;
};


IDESettings* IDESettings::getInstance(IDEWindow* window)
{
    if(instance==nullptr){
        instance = new IDESettings(window);
    }
    return instance;
}

void IDESettings::destory(){
    if(instance!=nullptr){
        delete instance;
    }
}

IDESettings::~IDESettings(){
    delete d;
}

bool IDESettings::readFromFile(const QString& filename){
    const QString appPath = QCoreApplication::applicationDirPath();
    QFile file(appPath+"/settings.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //qDebug() << "can not write";
        return false;
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(in.readAll().toUtf8(),&error);
    if(error.error==QJsonParseError::NoError){
        QJsonObject data = doc.object();
        {
            auto v = data.take("version");
            if(!v.isUndefined()){
                d->version = v.toInt(1);
            }
        }

        {
            auto v = data.take("maximized");
            if(!v.isUndefined()){
                d->maximized = v.toInt(0);
            }
        }
        {
            auto v = data.take("width");
            if(!v.isUndefined()){
                d->width = v.toInt(800);
            }
        }
        {
            auto v = data.take("height");
            if(!v.isUndefined()){
                d->height = v.toInt(600);
            }
        }
        {
            auto v = data.take("dockpanes");
            if(v.isObject()){
                d->dockpanes = v.toObject();
            }
        }

        {
            auto v = data.take("projects");
            if(v.isArray()){
                d->projects = v.toArray();
            }
        }
        return true;
    }else{
        qDebug()<<"json parse error:"<<error.errorString()<<";offset:"<<error.offset;
        return false;
    }

}

bool IDESettings::saveToFile(const QString& filename){
    int width = 0;
    int height = 0;
    int maximized = 0;
    if(d->window!=nullptr){
        width = d->window->width();
        height = d->window->height();
        maximized = d->window->isMaximized()?1:0;
    }
    QJsonObject data = {
        {"version",d->version},
        {"width",width},
        {"height",height},
        {"maximized",maximized},
        {"dockpanes",d->dockpanes},
        {"projects",d->projects}
    };

    QJsonDocument doc;
    doc.setObject(data);
    //QByteArray bytes = doc.toJson(QJsonDocument::Indented);
    const QString appPath = QCoreApplication::applicationDirPath();
    if(filename.isEmpty()){
        //settings.json

    }

    QFile file(appPath+"/settings.json");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        //qDebug() << "can not write";
        return false;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out <<doc.toJson(QJsonDocument::Indented);
    file.close();
    return true;
}



void IDESettings::setDockpanes(const QJsonObject& dockpanes){

    d->dockpanes = dockpanes;
}

void IDESettings::setProjects(const QJsonArray& projects){
    d->projects = projects;
}

QJsonObject IDESettings::dockpanes(){
    return d->dockpanes;
}

QJsonArray IDESettings::projects(){
    return d->projects;
}

bool IDESettings::isMaximized(){
    return d->maximized==1;
}

int IDESettings::width(){
    return d->width;
}

int IDESettings::height(){
    return d->height;
}

IDESettings::IDESettings(IDEWindow* window)
{
    d = new IDESettingsPrivate;
    d->window = window;
}

}
