#include "version_controller_thread.h"
#include "core/backend_thread.h"

namespace ady{
class VersionControllerThreadPrivate{
public:
    //BackendThreadTask* task;
    QList<BackendThreadTask*> tasks;
};

VersionControllerThread::VersionControllerThread(BackendThreadTask* task,QObject *parent):QThread(parent) {
    d = new VersionControllerThreadPrivate;
    d->tasks << task;
}


VersionControllerThread::VersionControllerThread(QList<BackendThreadTask*>& tasks,QObject *parent):QThread(parent) {
    d = new VersionControllerThreadPrivate;
    d->tasks = tasks;
}


VersionControllerThread::~VersionControllerThread(){
    //delete d->task;
    qDeleteAll(d->tasks);
    delete d;
}


void VersionControllerThread::run(){
    for(auto task:d->tasks){
        task->exec();
    }
}


}


