#include "resource_manage_read_folder_task.h"
#include "resource_manager_model.h"
#include <QDir>
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

bool ResourceManageReadFolderTask::exec(){
    int type = this->type();
    auto model = this->model();
    auto path = this->path();
    QDir dir(path);
    if (!dir.exists()) {
        return false;
    }
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name|QDir::DirsFirst|QDir::IgnoreCase);
    if(model!=nullptr){
        emit model->updateChildren(list,path,type);
    }
    return true;
}

ResourceManagerModel* ResourceManageReadFolderTask::model(){
    return d->model;
}

QString ResourceManageReadFolderTask::path(){
    return d->path;
}

}


