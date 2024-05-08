#ifndef SITESETTING_H
#define SITESETTING_H
#include "global.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>
namespace ady{
    class ANYENGINE_EXPORT SiteSetting
    {

    public:
        SiteSetting(QString json=QString());
        bool load(QString json);

        QJsonValue operator[] (const QString& name);
        QJsonValue operator[] (QString name);
        QJsonValue operator[] (char* name);
        QJsonValue get(const QString& name);
        void set(QString name,QJsonValue value);
        void set(QString name,int value);
        void set(QString name,QString value);
        void set(QString name,QStringList value);
        void set(QString name,bool value);
        void set(QString name,QJsonObject value);
        void remove(QString name);

        inline QJsonParseError error(){return m_error;}

        QString toJSON();

    private:
        QJsonDocument m_doc;
        QJsonObject m_object;
        QJsonParseError m_error;

    };

}
#endif // SITESETTING_H
