#include "subscriber.h"
#include "publisher.h"


namespace ady{
Subscriber::Subscriber()
{

}

void Subscriber::reg(){
    Publisher::getInstance()->reg(this);
}

void Subscriber::unReg(){
    Publisher::getInstance()->unReg(this);
}

}
