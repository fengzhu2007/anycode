#include "schedule.h"
#include "schedule_task.h"
#include "auto_save_task.h"
#include <QTimer>
#include <QDateTime>
#include <QDebug>
namespace ady{
Schedule* Schedule::instance = nullptr;
class SchedulePrivate{
public:
    QTimer timer;
    QList<ScheduleTask*> queue;
};

Schedule::Schedule(QObject* parent):QObject(parent) {
    d = new SchedulePrivate;
    connect(&d->timer,&QTimer::timeout,this,&Schedule::onTimeout);

    d->timer.setInterval(1*1000);//1 second
    //d->timer.start();
}

Schedule::~Schedule(){
    qDeleteAll(d->queue);
    delete d;
}

Schedule * Schedule::getInstance(){
    return instance;
}

void Schedule::init(QObject* parent){
    if(instance==nullptr){
        instance = new Schedule(parent);
    }
}

void Schedule::stop(){
    instance->d->timer.stop();
}

void Schedule::start(){
    instance->d->timer.start();
}

void Schedule::addAutoSave(int msec){
    d->queue<<(new AutoSaveTask(msec));
}

void Schedule::onTimeout(){
    long long mtime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    for(auto one:d->queue){
        if(one->isShouldExecute(mtime)){
            one->doing();
            one->execute();
            one->done();
        }
    }
}


}
