#include "version_control_query_diff_task.h"

#include "cvs/repository.h"


namespace ady{
class VersionControlQueryDiffTaskPrivate{
public:
    QPair<QString,QString> oid;

};

VersionControlQueryDiffTask::VersionControlQueryDiffTask(cvs::Repository* repo,const QString& oid1,const QString& oid2)
    :BackendThreadTask(BackendThreadTask::QueryDiff,(void*)repo)
{
    repo->used();//add count number;
    d = new VersionControlQueryDiffTaskPrivate;
    d->oid.first = oid1;
    d->oid.second = oid2;
}

VersionControlQueryDiffTask::~VersionControlQueryDiffTask(){
    delete d;
}

cvs::Repository* VersionControlQueryDiffTask::repository(){
    return static_cast<cvs::Repository*>(BackendThreadTask::data());
}

QPair<QString,QString> VersionControlQueryDiffTask::oid(){
    return d->oid;
}

}
