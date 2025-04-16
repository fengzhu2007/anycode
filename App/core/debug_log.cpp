#include "debug_log.h"
#include <QFile>
#include <QDateTime>

namespace ady{

DebugLog* DebugLog::instance = nullptr;;

class DebugLogPrivate{
public:
    QFile* log;
    bool state;

};

DebugLog::DebugLog() {
    d = new DebugLogPrivate();
    d->log = new QFile("log.txt");

}

DebugLog::~DebugLog() {
    if(d->state && d->log!=nullptr && d->log->isOpen()){
        d->log->close();
    }
    delete d;
}

DebugLog* DebugLog::getInstance(){
    if(instance==nullptr){
        instance = new DebugLog();
    }
    return instance;
}

void DebugLog::destory(){
    if(instance!=nullptr){
        delete instance;
        instance = nullptr;
    }
}

void DebugLog::write(const QString& category,const QString& content){
    if(instance==nullptr){
        getInstance();
    }
    if(instance->d->log->open(QIODevice::WriteOnly|QIODevice::Append)){
        instance->d->state = true;
    }else{
        instance->d->state = false;
        return ;
    }
    auto line = QString::fromUtf8("%1:%2 %3\n").arg(category).arg(QDateTime::currentDateTime().toString()).arg(content).toStdString();
    instance->d->log->write(line.data());
    instance->d->log->close();
}

}
