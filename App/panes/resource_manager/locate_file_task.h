#ifndef LOCATE_FILE_TASK_H
#define LOCATE_FILE_TASK_H
#include "core/backend_thread.h"

namespace ady{
class ResourceManagerModelItem;
class ResourceManagerModel;
class LocateFileTaskPrivate;
class LocateFileTask : public BackendThreadTask
{
public:
    LocateFileTask(ResourceManagerModel* model,const QString& from,const QString& path);
    virtual ~LocateFileTask();
    virtual bool exec() override;
    ResourceManagerModel* model();
    QString path();
private:
    LocateFileTaskPrivate* d;
};
}

#endif // LOCATE_FILE_TASK_H
