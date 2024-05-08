#include "{TPL}Setting.h"
#include "{TPL}SettingKey.h"
#include <QJsonArray>
#include <QJsonObject>
namespace ady {

{TPL}Setting::{TPL}Setting(SiteSetting & setting){
    {
        QJsonValue value = setting.get({TPL}_LOCAL_DIR_SYNC);
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
            m_data.insert({TPL}_LOCAL_DIR_SYNC,ptr);
        }
    }


    {
        QJsonValue value = setting.get({TPL}_REMOTE_DIR_MAPPING);
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
            m_data.insert({TPL}_REMOTE_DIR_MAPPING,ptr);
        }
    }



}

{TPL}Setting::~{TPL}Setting()
{
    QMap<QString,void*>::iterator iter = m_data.begin();
    while(iter!=m_data.end()){
        QString key = iter.key();
        if(key=={TPL}_LOCAL_DIR_SYNC || key=={TPL}_REMOTE_DIR_MAPPING){
            delete (QList<QPair<QString,QString>>*)iter.value();
        }
        iter++;
    }
}

QList<QPair<QString,QString>> {TPL}Setting::dirSync()
{
    if(m_data.contains({TPL}_LOCAL_DIR_SYNC)){
        QList<QPair<QString,QString>>* ptr = (QList<QPair<QString,QString>>*)m_data[{TPL}_LOCAL_DIR_SYNC];
        return *ptr;
    }else{
        return QList<QPair<QString,QString>>();
    }
}

QList<QPair<QString,QString>> {TPL}Setting::dirMapping()
{
    if(m_data.contains({TPL}_REMOTE_DIR_MAPPING)){
        QList<QPair<QString,QString>>* ptr = (QList<QPair<QString,QString>>*)m_data[{TPL}_REMOTE_DIR_MAPPING];
        return *ptr;
    }else{
        return QList<QPair<QString,QString>>();
    }
}

}
