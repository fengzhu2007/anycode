#ifndef PUBLISHER_H
#define PUBLISHER_H
#include "global.h"
#include <QObject>
#include <QJsonValue>
namespace ady{
class Event;
class PublisherPrivate;
class Subscriber;
class ANYENGINE_EXPORT Publisher : public QObject
{
    Q_OBJECT
public:
    static Publisher* getInstance();
    static void destory();
    void post(Event* e);
    void post(const QString& id);
    void post(const QString&id,void* data);
    void post(const QString&id,QJsonValueRef& data);
    void reg(Subscriber* subscriber);
    void unReg(Subscriber* subscriber);

private:
    static Publisher* instance;
    PublisherPrivate* d;

private:
    Publisher();
};

}

#endif // PUBLISHER_H
