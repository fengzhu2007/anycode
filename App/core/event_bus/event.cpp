#include "event.h"

namespace ady{

class EventPrivate{
public:
    QString id;
    void *data;
    bool ignore=false;
    QJsonValue jsonData ;
};

Event::Event(const QString& id)
{
    d = new EventPrivate;
    d->id = id;
    d->data = nullptr;
}

Event::Event(const QString& id,void *data){
    d = new EventPrivate;
    d->id = id;
    d->data = data;
}

Event::Event(const QString& id,QJsonValueRef& data){
    d = new EventPrivate;
    d->id = id;
    d->data = nullptr;
    d->jsonData = data;
}

Event::~Event(){
    delete d;
}


void Event::ignore(){
    d->ignore = true;
}

bool Event::isIgnore(){
    return d->ignore;
}


bool Event::isJsonData(){
    return !(d->jsonData.isUndefined() || d->jsonData.isNull());
}

QJsonValue Event::jsonData(){
    return d->jsonData;
}

const QString Event::id(){
    return d->id;
}

void* Event::data(){
    return d->data;
}

}
