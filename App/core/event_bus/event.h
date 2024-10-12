#ifndef EVENT_H
#define EVENT_H
#include "global.h"
#include <QString>
#include <QJsonValue>
namespace ady{

class EventPrivate;
class ANYENGINE_EXPORT Event
{
public:
    explicit Event(const QString& id);
    explicit Event(const QString& id,void *data);
    explicit Event(const QString& id,QJsonValueRef& data);
    virtual ~Event();
    void ignore();
    bool isIgnore();
    bool isJsonData();
    QJsonValue jsonData();

    const QString id();
    void* data();



private:
    EventPrivate* d;

};

}



#endif // EVENT_H
