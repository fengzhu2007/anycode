#include "cos_setting.h"
#include "cos_setting_key.h"
#include <QJsonArray>
#include <QJsonObject>
namespace ady {

COSSetting::COSSetting(const SiteSetting & setting){
    {
        QJsonValue value = setting.get(COS_LOCAL_DIR_SYNC);
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
            m_data.insert(COS_LOCAL_DIR_SYNC,ptr);
        }
    }


    {
        QJsonValue value = setting.get(COS_REMOTE_DIR_MAPPING);
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
            m_data.insert(COS_REMOTE_DIR_MAPPING,ptr);
        }
    }

    {
        QJsonValue value = setting.get(COS_FILTER_DIRS);
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
            m_data.insert(COS_FILTER_DIRS,ptr);
        }
    }

    {
        QJsonValue value = setting.get(COS_FILTER_EXTENSIONS);
        if(!value.isNull() && value.toString().isEmpty()==false){
            QStringList* ptr = new QStringList;
            *ptr = value.toString().split(",");
            m_data.insert(COS_FILTER_EXTENSIONS,ptr);
        }
    }


}

COSSetting::~COSSetting()
{
    QMap<QString,void*>::iterator iter = m_data.begin();
    while(iter!=m_data.end()){
        QString key = iter.key();
        if(key==COS_LOCAL_DIR_SYNC || key==COS_REMOTE_DIR_MAPPING){
            delete (QList<QPair<QString,QString>>*)iter.value();
        }
        iter++;
    }
}

QList<QPair<QString,QString>> COSSetting::dirSync()
{
    if(m_data.contains(COS_LOCAL_DIR_SYNC)){
        QList<QPair<QString,QString>>* ptr = (QList<QPair<QString,QString>>*)m_data[COS_LOCAL_DIR_SYNC];
        return *ptr;
    }else{
        return QList<QPair<QString,QString>>();
    }
}

QList<QPair<QString,QString>> COSSetting::dirMapping()
{
    if(m_data.contains(COS_REMOTE_DIR_MAPPING)){
        QList<QPair<QString,QString>>* ptr = (QList<QPair<QString,QString>>*)m_data[COS_REMOTE_DIR_MAPPING];
        return *ptr;
    }else{
        return QList<QPair<QString,QString>>();
    }
}

QStringList COSSetting::filterExtensions()
{
    if(m_data.contains(COS_FILTER_EXTENSIONS)){
        QStringList* ptr = (QStringList*)m_data[COS_FILTER_EXTENSIONS];
        return *ptr;
    }else{
        return QStringList();
    }
}

QStringList COSSetting::filterDirs(){

    if(m_data.contains(COS_FILTER_DIRS)){
        QStringList* ptr = (QStringList*)m_data[COS_FILTER_DIRS];
        return *ptr;
    }else{
        return QStringList();
    }
}

}
