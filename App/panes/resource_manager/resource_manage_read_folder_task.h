#ifndef RESOURCEMANAGEREADFOLDERTASK_H
#define RESOURCEMANAGEREADFOLDERTASK_H
#include "core/backend_thread.h"
namespace ady{
class ResourceManagerModelItem;
class ResourceManagerModel;
class ResourceManageReadFolderTaskPrivate;
class ResourceManageReadFolderTask : public BackendThreadTask
{
public:
    ResourceManageReadFolderTask(ResourceManagerModel* model,const QString& path);
    virtual ~ResourceManageReadFolderTask();
    virtual bool exec() override;
    ResourceManagerModel* model();
    QString path();
private:
    ResourceManageReadFolderTaskPrivate* d;
};
}
#endif // RESOURCEMANAGEREADFOLDERTASK_H
