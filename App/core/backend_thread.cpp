#include "backend_thread.h"




#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>

namespace ady{
BackendThread* BackendThread::instance = nullptr;

class BackendThreadTaskPrivate{
public:
    int type;
    void *data = nullptr;

};

BackendThreadTask::BackendThreadTask(int type){
    d = new BackendThreadTaskPrivate;
    d->type = type;
}

BackendThreadTask::BackendThreadTask(int type,void* data){
    d = new BackendThreadTaskPrivate;
    d->type = type;
    d->data = data;
}

BackendThreadTask::~BackendThreadTask(){
    delete d;
}


int BackendThreadTask::type(){
    return d->type;
}
void* BackendThreadTask::data(){
    return d->data;
}

void BackendThreadTask::setType(int type){
    d->type = type;
}

/******************BackendThread start**********************/


class BackendThreadPrivate{
public:
    QList<BackendThreadTask*> list;
    bool stopped = false;
    QMutex mutex;
    QWaitCondition condition;
    BackendThreadTask* current = nullptr;
};

BackendThread::BackendThread(QObject *parent)
    : QThread{parent}
{
    d = new BackendThreadPrivate;
}
BackendThread* BackendThread::getInstance(){
    init();
    return instance;
}

BackendThread* BackendThread::init(){
    if(instance==nullptr){
        instance = new BackendThread();
    }
    return instance;
}

BackendThread::~BackendThread(){
    delete d;
}
BackendThread* BackendThread::stop(){
    d->stopped = true;
    qDeleteAll(d->list);
    return this;
}

void BackendThread::appendTask(BackendThreadTask* task){
    QMutexLocker locker(&d->mutex);
    d->list.push_back(task);
    d->condition.wakeOne();
}

/*void BackendThread::appendTask(BackendThreadTask::Type type,void* data){
    this->appendTask(new BackendThreadTask(type,data));
}*/

void BackendThread::run(){
    while(d->stopped==false){
        d->current = takeFirst();
        if(d->current!=nullptr){
            bool ret = d->current->exec();
            /*switch(d->current->type()){
            case BackendThreadTask::ReadFolder:
                this->doReadFolder();
                goto break_switch;
            case BackendThreadTask::ReadFile:
                this->doReadFile();
                goto break_switch;
            case BackendThreadTask::RefreshFolder:
                this->doReadFolder(true);
                goto break_switch;
            case BackendThreadTask::QueryCommit:
                this->doQueryCommit();
                goto break_switch;
            case BackendThreadTask::QueryDiff:
                this->doQueryDiff();
                goto break_switch;
            }
break_switch:*/
            delete d->current;
            d->current = nullptr;
        }
    }
}

BackendThreadTask* BackendThread::takeFirst(){
    QMutexLocker locker(&d->mutex);
    while(d->stopped==false){
        if(d->list.size()==0){
            d->condition.wait(&d->mutex,5*1000);
        }else{
            return d->list.takeFirst();
        }
    }
    return nullptr;
}

//do task
/*void BackendThread::doReadFolder(bool refresh){
    auto task = static_cast<ResourceManageReadFolderTask*>(d->current);
    auto model = task->model();
    auto path = task->path();
        QDir dir(path);
        if (!dir.exists()) {
            return ;
        }
        QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name|QDir::DirsFirst|QDir::IgnoreCase);
        if(model!=nullptr){
            emit model->updateChildren(list,path,refresh);
        }

}

void BackendThread::doReadFile(){
    auto task = static_cast<CodeEditorReadFileTask*>(d->current);
    auto instance = CodeEditorManager::getInstance();
    if(instance!=nullptr){
        instance->readFileLines(task->path());
    }
}

void BackendThread::doQueryCommit(){
    auto task = static_cast<VersionControlQueryCommitTask*>(d->current);
    auto repo = task->repository();
    auto list = repo->queryCommit(100);
    auto instance = VersionControlPane::getInstance();
    if(instance!=nullptr){
        QMetaObject::invokeMethod(instance,"onUpdateCommit", Qt::AutoConnection,Q_ARG(void*,repo),Q_ARG(void*, list));
    }else{
        delete list;
        repo->unUsed();
    }
}

void BackendThread::doQueryDiff(){
    auto task = static_cast<VersionControlQueryDiffTask*>(d->current);
    auto repo = task->repository();
    auto oid = task->oid();
    auto list = repo->queryDiff(oid.first,oid.second);
    auto instance = VersionControlPane::getInstance();
    if(instance!=nullptr){
        QMetaObject::invokeMethod(instance,"onUpdateDiff", Qt::AutoConnection,Q_ARG(void*,repo),Q_ARG(void*, list));
    }else{
        delete list;
        repo->unUsed();
    }
}*/


}
