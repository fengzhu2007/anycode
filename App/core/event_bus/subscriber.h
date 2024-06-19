#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H
#include "global.h"

namespace ady{
class Event;
class ANYENGINE_EXPORT Subscriber
{
public:
    Subscriber();
    virtual void reg();
    virtual void unReg();
    virtual bool onReceive(Event*)=0;

};


}

#endif // SUBSCRIBER_H
