#include "schedule.h"
#include "schedule_task.h"
#include "file_auto_save_task.h"
#include "network_auto_close_task.h"
#include "network_rate_task.h"
#include "network_status_task.h"
#include "tools/ssl_query/ssl_query_task.h"
#include "storage/common_storage.h"
#include <QTimer>
#include <QDateTime>
#include <QDebug>
namespace ady{
Schedule* Schedule::instance = nullptr;
class SchedulePrivate{
public:
    QTimer timer;
    QList<ScheduleTask*> queue;
    FileAutoSaveTask* file_auto_save_task;
    NetworkAutoCloseTask* network_auto_close_task;
    NetworkStatusTask* network_status_task;
    long long start_time;//m second

};

Schedule::Schedule(QObject* parent):QObject(parent) {
    d = new SchedulePrivate;
    d->file_auto_save_task = nullptr;
    d->network_auto_close_task = nullptr;
    d->network_status_task = nullptr;
    d->start_time = QDateTime::currentDateTime().toMSecsSinceEpoch();
    //d->file_auto_save_task = nullptr;
    connect(&d->timer,&QTimer::timeout,this,&Schedule::onTimeout);

    d->timer.setInterval(1*1000);//1 second
    //add network rate task
    d->queue << new NetworkRateTask(1*1000);
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

void Schedule::addFileAutoSave(int msec){
    if(d->file_auto_save_task==nullptr){
        d->queue<<(d->file_auto_save_task = new FileAutoSaveTask(msec));
    }else{
        d->file_auto_save_task->m_interval = msec;
    }
}



void Schedule::addNetworkAutoClose(int msec){
    if(d->file_auto_save_task==nullptr){
        d->queue<<(d->network_auto_close_task = new NetworkAutoCloseTask(msec));
    }else{
        d->network_auto_close_task->m_interval = msec;
    }
}

void Schedule::addNetworkStatusWatching(int msec){
    if(d->network_status_task==nullptr){
        d->queue<<(d->network_status_task = new NetworkStatusTask(msec));
    }
}

void Schedule::addSSLQuery(){
    auto r = CommonStorage().one(SSLQueryTask::SSL_QUERIER_KEY);
    if(!r.value.isEmpty()){
        QStringList sites = r.value.split(",");
        d->queue << (new SSLQueryTask(sites,SSLQueryTask::DELAY));
    }
}

void Schedule::onTimeout(){
    long long mtime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    long long duration = mtime - d->start_time;
    for(auto one:d->queue){
        if(one->isShouldExecute(mtime,duration)){
            one->doing();
            one->execute();
            one->done();
            if(one->isOnce()){
                d->queue.removeOne(one);
                delete one;
            }
        }
    }
}

void Schedule::addTask(ScheduleTask* task){
    d->queue<<task;
}


}
