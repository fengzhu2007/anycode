#include "oss_setting.h"
#include "oss_setting_key.h"
#include <QJsonArray>
#include <QJsonObject>
namespace ady {

OSSSetting::OSSSetting(const SiteSetting & setting){
    {
        QJsonValue value = setting.get(OSS_LOCAL_DIR_SYNC);
        if(!value.isNull() && value.toArray().size()>0){
            QList<QPair<QString,QString>>* ptr = new QList<QPair<QString,QString>>();
            QJsonArray arr = value.toArray();
            for(auto one:arr){
                if(!one.isNull()){
                    QJsonObject object = one.toObject();
                    if(object.contains(SRC) && object.contains(DST)){
                        ptr->push_back(qMakePair(object.value(SRC).toString(),object.value(DST).toString()));
                    }
                }
            }
            m_data.insert(OSS_LOCAL_DIR_SYNC,ptr);
        }
    }


    {
        QJsonValue value = setting.get(OSS_REMOTE_DIR_MAPPING);
        if(!value.isNull() && value.toArray().size()>0){
            QList<QPair<QString,QString>>* ptr = new QList<QPair<QString,QString>>();
            QJsonArray arr = value.toArray();
            for(auto one:arr){
                if(!one.isNull()){
                    QJsonObject object = one.toObject();
                    if(object.contains(SRC) && object.contains(DST)){
                        ptr->push_back(qMakePair(object.value(SRC).toString(),object.value(DST).toString()));
                    }
                }
            }
            m_data.insert(OSS_REMOTE_DIR_MAPPING,ptr);
        }
    }



    {
        QJsonValue value = setting.get(OSS_FILTER_DIRS);
        if(!value.isNull() && value.toArray().size()>0){
            //QList<QPair<QString,QString>>* ptr = new QList<QPair<QString,QString>>();
            QStringList* ptr = new QStringList;
            QJsonArray arr = value.toArray();
            for(auto one:arr){
                if(!one.isNull()){
                    QJsonObject object = one.toObject();
                    if(object.contains(SRC) /*&& object.contains(DST)*/){
                        //ptr->push_back(qMakePair(object.value(SRC).toString(),object.value(DST).toString()));
                        ptr->push_back(object.value(SRC).toString());
                    }
                }
            }
            m_data.insert(OSS_FILTER_DIRS,ptr);
        }
    }

    {
        QJsonValue value = setting.get(OSS_FILTER_EXTENSIONS);
        if(!value.isNull() && value.toString().isEmpty()==false){
            QStringList* ptr = new QStringList;
            *ptr = value.toString().split(",");
            m_data.insert(OSS_FILTER_EXTENSIONS,ptr);
        }
    }


}

OSSSetting::~OSSSetting()
{
    QMap<QString,void*>::iterator iter = m_data.begin();
    while(iter!=m_data.end()){
        QString key = iter.key();
        if(key==OSS_LOCAL_DIR_SYNC || key==OSS_REMOTE_DIR_MAPPING){
            delete (QList<QPair<QString,QString>>*)iter.value();
        }else if(key==OSS_FILTER_EXTENSIONS || key==OSS_FILTER_DIRS){
            delete (QStringList*)iter.value();
        }
        iter++;
    }
}

QList<QPair<QString,QString>> OSSSetting::dirSync()
{
    if(m_data.contains(OSS_LOCAL_DIR_SYNC)){
        QList<QPair<QString,QString>>* ptr = (QList<QPair<QString,QString>>*)m_data[OSS_LOCAL_DIR_SYNC];
        return *ptr;
    }else{
        return QList<QPair<QString,QString>>();
    }
}

QList<QPair<QString,QString>> OSSSetting::dirMapping()
{
    if(m_data.contains(OSS_REMOTE_DIR_MAPPING)){
        QList<QPair<QString,QString>>* ptr = (QList<QPair<QString,QString>>*)m_data[OSS_REMOTE_DIR_MAPPING];
        return *ptr;
    }else{
        return QList<QPair<QString,QString>>();
    }
}

QStringList OSSSetting::filterExtensions()
{
    if(m_data.contains(OSS_FILTER_EXTENSIONS)){
        QStringList* ptr = (QStringList*)m_data[OSS_FILTER_EXTENSIONS];
        return *ptr;
    }else{
        return QStringList();
    }
}

QStringList OSSSetting::filterDirs(){

    if(m_data.contains(OSS_FILTER_DIRS)){
        QStringList* ptr = (QStringList*)m_data[OSS_FILTER_DIRS];
        return *ptr;
    }else{
        return QStringList();
    }
}

}
