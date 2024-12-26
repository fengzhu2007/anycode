#include "locate_file_task.h"
#include "resource_manager_model.h"
#include <QDir>

namespace ady{
class LocateFileTaskPrivate{
public:
    ResourceManagerModel* model;
    QString from;
    QString path;

};

LocateFileTask::LocateFileTask(ResourceManagerModel* model,const QString& from,const QString& path)
    :BackendThreadTask(BackendThreadTask::LocateFile) {
    d = new LocateFileTaskPrivate;
    d->model = model;
    d->from = from;
    d->path = path;
}

LocateFileTask::~LocateFileTask(){
    delete d;
}


static int find(const QFileInfoList& list,const QString& path){
    int i = -1;
    for(auto one:list){
        i++;
        if(one.isFile() && one.absoluteFilePath() == path){
            return i;
        }else if(one.isDir() && path.startsWith(one.absoluteFilePath()+"/")){
            return i;
        }
    }
    return -1;
}

bool LocateFileTask::exec(){
    QDir dir(d->from);
    if (!dir.exists()) {
        return false;
    }
    while(true){
        QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name|QDir::DirsFirst|QDir::IgnoreCase);
        if(list.size()==0){
            break;
        }

        int i = find(list,d->path);
        if(i>-1){
            auto one = list.at(i);
            emit d->model->updateChildren(list,dir.absolutePath(),BackendThreadTask::ReadFolder);
            if(one.isFile()){
                //find ok
                emit d->model->locateSuccess(d->path,false);
                return true;
            }else if(one.isDir()){
                //find dir and append item to the model
                dir.cd(one.baseName());//cd child dir
            }else{
                break;
            }
        }else{
            break;
        }
        if(QThread::currentThread()->isInterruptionRequested()){
            break;
        }
    }
    return true;
}

ResourceManagerModel* LocateFileTask::model(){
    return d->model;
}

QString LocateFileTask::path(){
    return d->path;
}

}
