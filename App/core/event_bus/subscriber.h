#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H
#include "global.h"
#include <QStringList>

namespace ady{
class Event;
class SubscriberPrivate;
class ANYENGINE_EXPORT Subscriber
{
public:
    Subscriber();
    virtual ~Subscriber();
    virtual void reg();
    virtual void unReg();
    virtual bool onReceive(Event*)=0;
    void regMessageId(const QString& id);
    void regMessageIds(QStringList ids);
    void unRegMessageId(const QString& id);
    void unRegAllMessage();

    virtual bool distributeCheck(Event* e);
private:
    SubscriberPrivate* d;

};


}

#endif // SUBSCRIBER_H
