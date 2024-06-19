#ifndef EVENT_H
#define EVENT_H
#include "global.h"
#include <QString>
namespace ady{

class EventPrivate;
class ANYENGINE_EXPORT Event
{
public:
    Event(const QString& id);
    Event(const QString& id,void *data);
    ~Event();

    const QString id();


private:
    EventPrivate* d;

};

}



#endif // EVENT_H
