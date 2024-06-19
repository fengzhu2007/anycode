#include "event.h"

namespace ady{

class EventPrivate{
public:
    QString id;
    void *data;
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

Event::~Event(){
    delete d;
}

const QString Event::id(){
    return d->id;
}
}
