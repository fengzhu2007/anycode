#ifndef VERSIONCONTROLQUERYDIFFTASK_H
#define VERSIONCONTROLQUERYDIFFTASK_H
#include "core/backend_thread.h"
namespace ady{
namespace cvs {
class Repository;
}

class VersionControlQueryDiffTaskPrivate;
class VersionControlQueryDiffTask : public BackendThreadTask
{
public:
    VersionControlQueryDiffTask(cvs::Repository* repo,const QString& oid1={},const QString& oid2={});
    ~VersionControlQueryDiffTask();
    cvs::Repository* repository();
    QPair<QString,QString> oid();
private:
    VersionControlQueryDiffTaskPrivate* d;
};
}



#endif // VERSIONCONTROLQUERYDIFFTASK_H
