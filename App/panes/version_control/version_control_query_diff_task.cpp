#include "version_control_query_diff_task.h"
#include "version_control_pane.h"
#include "cvs/repository.h"
#include <QDebug>

namespace ady{
class VersionControlQueryDiffTaskPrivate{
public:
    QPair<QString,QString> oid;
    std::shared_ptr<cvs::Repository> repo;

};

VersionControlQueryDiffTask::VersionControlQueryDiffTask(std::shared_ptr<cvs::Repository>& repo,const QString& oid1,const QString& oid2)
    :BackendThreadTask(BackendThreadTask::QueryDiff)
{
    d = new VersionControlQueryDiffTaskPrivate;
    d->repo = repo;
    d->oid.first = oid1;
    d->oid.second = oid2;
}

VersionControlQueryDiffTask::~VersionControlQueryDiffTask(){
    delete d;
}

bool VersionControlQueryDiffTask::exec(){
    auto oid = this->oid();
    auto list = d->repo->queryDiff(oid.first,oid.second);
    auto instance = VersionControlPane::getInstance();
    if(instance!=nullptr){
        QMetaObject::invokeMethod(instance,"onUpdateDiff", Qt::AutoConnection,Q_ARG(const QString&,oid.first),Q_ARG(const QString&,oid.second),Q_ARG(int,d->repo->rid()),Q_ARG(void*, list));
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


QPair<QString,QString> VersionControlQueryDiffTask::oid(){
    return d->oid;
}

}
