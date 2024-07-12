#ifndef VERSIONCONTROLQUERYCOMMITTASK_H
#define VERSIONCONTROLQUERYCOMMITTASK_H
#include "core/backend_thread.h"
namespace ady{
namespace cvs {
class Repository;
}

class VersionControlQueryCommitTaskPrivate;
class VersionControlQueryCommitTask : public BackendThreadTask
{
public:
    VersionControlQueryCommitTask(cvs::Repository* repo);
    ~VersionControlQueryCommitTask();
    cvs::Repository* repository();
private:
    VersionControlQueryCommitTaskPrivate* d;
};
}
#endif // VERSIONCONTROLQUERYCOMMITTASK_H
