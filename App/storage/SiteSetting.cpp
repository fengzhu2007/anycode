#include "SiteSetting.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
namespace ady{

SiteSetting::SiteSetting(QString json)
{
    if(!json.isEmpty()){
        this->load(json);
    }
}

bool SiteSetting::load(QString json)
{
    m_doc = QJsonDocument::fromJson(json.toUtf8(),&m_error);
    if(m_error.error == QJsonParseError::NoError){
        m_object = m_doc.object();
        return true;
    }else{
        return false;
    }
}

void SiteSetting::set(QString name,QJsonValue value)
{
    m_object.insert(name,value);
}

void SiteSetting::remove(QString name)
{
    m_object.remove(name);
}

QJsonValue SiteSetting::operator[] (const QString& name)
{
    return this->get(name);
}

QJsonValue SiteSetting::operator[] (QString name)
{
    return this->get(name);
}

QJsonValue SiteSetting::operator[] (char* name)
{
    return this->get(name);
}

QJsonValue SiteSetting::get(const QString& name) const
{
    if(m_doc.isNull()){
        return QJsonValue();
    }else{
        //QJsonObject root = m_doc.object();
        if(m_object.contains(name)){
            return m_object.value(name);
        }else{
            return QJsonValue();
        }
    }
}

void SiteSetting::set(QString name,int value)
{
    set(name,QJsonValue(value));
}

void SiteSetting::set(QString name,QString value)
{
    set(name,QJsonValue(value));
}

void SiteSetting::set(QString name,QStringList value)
{
    QJsonArray array;
    for(auto v:value)
    {
        array.append(v);
    }
    set(name,array);
}

void SiteSetting::set(QString name,bool value)
{
    set(name,QJsonValue(value));
}

void SiteSetting::set(QString name,QJsonObject value)
{
    set(name,QJsonValue(value));
}

QString SiteSetting::toJSON()
{
    m_doc.setObject(m_object);
    //qDebug()<<"doc:"<<m_doc;
    return m_doc.toJson();
}



}
