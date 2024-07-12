#include "resource_manage_read_folder_task.h"

namespace ady{

class ResourceManageReadFolderTaskPrivate{
public:
    ResourceManagerModel* model;
    QString path;
};

ResourceManageReadFolderTask::ResourceManageReadFolderTask(ResourceManagerModel* model,const QString& path)
    :BackendThreadTask(BackendThreadTask::ReadFolder)
{
    d = new ResourceManageReadFolderTaskPrivate;
    d->model = model;
    d->path = path;
}

ResourceManageReadFolderTask::~ResourceManageReadFolderTask(){
    delete d;
}

ResourceManagerModel* ResourceManageReadFolderTask::model(){
    return d->model;
}

QString ResourceManageReadFolderTask::path(){
    return d->path;
}

}


