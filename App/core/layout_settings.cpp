#include "layout_settings.h"
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


LayoutSettings* LayoutSettings::instance = nullptr;


class LayoutSettingsPrivate{
public:
    int version=1;
    int width=800;
    int height = 600;
    int maximized = 0;
    QJsonObject dockpanes;
    QJsonArray projects;
    IDEWindow* window;
};


LayoutSettings* LayoutSettings::getInstance(IDEWindow* window)
{
    if(instance==nullptr){
        instance = new LayoutSettings(window);
    }
    return instance;
}

void LayoutSettings::destory(){
    if(instance!=nullptr){
        delete instance;
    }
}

LayoutSettings::~LayoutSettings(){
    delete d;
}

bool LayoutSettings::readFromFile(const QString& filename){
    const QString appPath = QCoreApplication::applicationDirPath();
    QFile file(appPath+"/layout.json");
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
        d->version = data.find("version")->toInt(1);
        d->maximized = data.find("maximized")->toInt(0);
        d->width = data.find("width")->toInt(800);
        d->height = data.find("height")->toInt(600);
        d->dockpanes = data.find("dockpanes")->toObject();
        d->projects = data.find("projects")->toArray();
        return true;
    }else{
        qDebug()<<"json parse error:"<<error.errorString()<<";offset:"<<error.offset;
        return false;
    }

}

bool LayoutSettings::saveToFile(const QString& filename){
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

    QFile file(appPath+"/layout.json");
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



void LayoutSettings::setDockpanes(const QJsonObject& dockpanes){

    d->dockpanes = dockpanes;
}

void LayoutSettings::setProjects(const QJsonArray& projects){
    d->projects = projects;
}

QJsonObject LayoutSettings::dockpanes(){
    return d->dockpanes;
}

QJsonArray LayoutSettings::projects(){
    return d->projects;
}

bool LayoutSettings::isMaximized(){
    return d->maximized==1;
}

int LayoutSettings::width(){
    return d->width;
}

int LayoutSettings::height(){
    return d->height;
}

LayoutSettings::LayoutSettings(IDEWindow* window)
{
    d = new LayoutSettingsPrivate;
    d->window = window;
}

}
