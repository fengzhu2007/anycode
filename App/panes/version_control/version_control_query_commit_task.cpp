#include "version_control_query_commit_task.h"
#include "cvs/repository.h"


namespace ady{
class VersionControlQueryCommitTaskPrivate{
public:



};

VersionControlQueryCommitTask::VersionControlQueryCommitTask(cvs::Repository* repo)
    :BackendThreadTask(BackendThreadTask::QueryCommit,(void*)repo)
{
    repo->used();//add count number;
}

VersionControlQueryCommitTask::~VersionControlQueryCommitTask(){

}

cvs::Repository* VersionControlQueryCommitTask::repository(){
    return static_cast<cvs::Repository*>(BackendThreadTask::data());
}
}
