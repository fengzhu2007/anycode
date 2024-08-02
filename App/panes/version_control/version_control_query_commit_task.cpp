#include "version_control_query_commit_task.h"
#include "version_control_pane.h"
#include "cvs/repository.h"
#include <QDebug>

namespace ady{
class VersionControlQueryCommitTaskPrivate{
public:
    std::shared_ptr<cvs::Repository> repo;


};

VersionControlQueryCommitTask::VersionControlQueryCommitTask(std::shared_ptr<cvs::Repository>& repo)
    :BackendThreadTask(BackendThreadTask::QueryCommit)
{
    d = new VersionControlQueryCommitTaskPrivate ;
    d->repo = repo;
}

VersionControlQueryCommitTask::~VersionControlQueryCommitTask(){
    delete d;
}

bool VersionControlQueryCommitTask::exec(){
    auto list = d->repo->queryCommit(100);
    auto instance = VersionControlPane::getInstance();
    //qDebug()<<"VersionControlQueryCommitTask:"<<list->size();
    if(instance!=nullptr){
        QMetaObject::invokeMethod(instance,"onUpdateCommit", Qt::AutoConnection,Q_ARG(int,d->repo->rid()),Q_ARG(void*, list));
        if(list->size()==0){
            auto error = d->repo->error();
            if(error.code!=0){
                QMetaObject::invokeMethod(instance,"onError", Qt::AutoConnection,Q_ARG(int,d->repo->rid()),Q_ARG(int,error.code),Q_ARG(const QString&, error.message));
            }
        }
        return true;
    }else{
        delete list;
        return false;
    }
}


}
