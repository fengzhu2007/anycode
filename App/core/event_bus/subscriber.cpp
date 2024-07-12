#include "subscriber.h"
#include "publisher.h"
#include "event.h"


namespace ady{
class SubscriberPrivate{
public:
    QStringList ids;
};

Subscriber::Subscriber()
{
    d = new SubscriberPrivate;
}

Subscriber::~Subscriber(){
    delete d;
}

void Subscriber::reg(){
    Publisher::getInstance()->reg(this);
}

void Subscriber::unReg(){
    Publisher::getInstance()->unReg(this);
}

void Subscriber::regMessageId(const QString& id){
    d->ids.push_back(id);
}

void Subscriber::unRegMessageId(const QString& id){
    d->ids.removeOne(id);
}

void Subscriber::regMessageIds(QStringList ids){
    d->ids.append(ids);
}

void Subscriber::unRegAllMessage(){
    d->ids.clear();
}

bool Subscriber::distributeCheck(Event* e){
    if(d->ids.contains(e->id())){
        return true;
    }else{
        return false;
    }
}

}
