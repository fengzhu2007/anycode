#include "dbms_thread.h"
#include <QDebug>
namespace ady{

class DBMSThreadPrivate{
public:
    int cmd;
    void* data;
    //bool interrupt = false;
};

DBMSThread::DBMSThread(int command,void* data,QObject *parent)
    : QThread{parent}
{
    d = new DBMSThreadPrivate;
    d->cmd = command;
    d->data = data;
}

DBMSThread::~DBMSThread(){
    delete d;
}

/*void ServerRequestThread::interrupt(){
    d->interrupt = true;
}*/

void DBMSThread::run(){

}



}
