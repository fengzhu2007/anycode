#ifndef PUBLISHER_H
#define PUBLISHER_H
#include <QObject>

namespace ady{
class Event;
class PublisherPrivate;
class Subscriber;
class Publisher : public QObject
{
    Q_OBJECT
public:
    static Publisher* getInstance();
    static void destory();
    void post(Event* e);
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
