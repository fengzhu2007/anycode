#include "publisher.h"
#include "event.h"
#include "subscriber.h"
#include <QList>

namespace ady{

Publisher* Publisher::instance = nullptr;


class PublisherPrivate{
public:
    QList<Subscriber*> subscribers;

};


Publisher* Publisher::getInstance(){
    if(instance==nullptr){
        instance = new Publisher();
    }
    return instance;
}

void Publisher::destory(){
    if(instance!=nullptr){
        delete instance;
    }

}

Publisher::Publisher()
{
    d = new PublisherPrivate;
}

void Publisher::post(Event* e){
    foreach(auto one,d->subscribers){
        if(one->distributeCheck(e)){
            one->onReceive(e);
            if(e->isIgnore()){
                break;
            }
        }
    }
    delete e;
}

void Publisher::post(const QString& id){
    this->post(new Event(id));
}

void Publisher::post(const QString&id,void* data){
    this->post(new Event(id,data));
}

void Publisher::reg(Subscriber* subscriber){
    d->subscribers.push_back(subscriber);
}

void Publisher::unReg(Subscriber* subscriber){
    d->subscribers.removeAll(subscriber);
}

}
