#ifndef VERSIONCONTROLQUERYCOMMITTASK_H
#define VERSIONCONTROLQUERYCOMMITTASK_H
#include "core/backend_thread.h"
#include <memory>
namespace ady{
namespace cvs {
class Repository;
}

class VersionControlQueryCommitTaskPrivate;
class VersionControlQueryCommitTask : public BackendThreadTask
{
public:
    VersionControlQueryCommitTask(std::shared_ptr<cvs::Repository>& repo);
    virtual ~VersionControlQueryCommitTask();
    virtual bool exec() override;
private:
    VersionControlQueryCommitTaskPrivate* d;
};
}
#endif // VERSIONCONTROLQUERYCOMMITTASK_H
