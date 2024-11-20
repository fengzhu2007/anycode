#ifndef VERSION_CONTROL_DELETE_FILE_TASK_H
#define VERSION_CONTROL_DELETE_FILE_TASK_H

#include "core/backend_thread.h"

namespace ady{

class VersionControlDeleteFileTaskPrivate;
class VersionControlDeleteFileTask : public BackendThreadTask
{
public:

    VersionControlDeleteFileTask(long long id,const QStringList& files);
    virtual ~VersionControlDeleteFileTask();
    virtual bool exec() override;
private:
    VersionControlDeleteFileTaskPrivate* d;
};

}


#endif // VERSION_CONTROL_DELETE_FILE_TASK_H
