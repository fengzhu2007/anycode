#include "version_control_delete_file_task.h"
#include "network/network_manager.h"
#include "network/network_response.h"
#include "version_control_pane.h"


namespace ady{

class VersionControlDeleteFileTaskPrivate{
public:
    long long id;
    QString projectPath;
    QString remoteRoot;
    QStringList files;
};



VersionControlDeleteFileTask::VersionControlDeleteFileTask(long long id,const QString& projectPath,const QString& remoteRoot,const QStringList& files)
    :BackendThreadTask(BackendThreadTask::Custome)
{
    d = new VersionControlDeleteFileTaskPrivate;
    d->id = id;
    d->projectPath = projectPath;
    d->remoteRoot = remoteRoot;
    d->files = files;
}


VersionControlDeleteFileTask::~VersionControlDeleteFileTask() {

    delete d;
}


bool VersionControlDeleteFileTask::exec() {
    auto manager = NetworkManager::getInstance();
    auto req = manager->request(d->id);
    if(req==nullptr){
        req = manager->initRequest(d->id,{});//lazily create http client if absent
    }
    if(req!=nullptr){
        auto instance = VersionControlPane::getInstance();
        if(!req->isConnected()){
            //establish connection before any command ,same as NetworkManager::exec
            auto linkResponse = req->link();
            bool linked = false;
            if(linkResponse!=nullptr){
                linked = linkResponse->status();
                if(instance!=nullptr){
                    QMetaObject::invokeMethod(instance,"onOutput", Qt::AutoConnection,Q_ARG(void*,linkResponse));
                }else{
                    delete linkResponse;
                }
            }
            if(!linked){
                return false;
            }
        }
        //convert local absolute path to remote path ,same as upload job
        const QString projectPath = d->projectPath.endsWith("/")?d->projectPath:d->projectPath+"/";
        QString remoteRoot = d->remoteRoot;
        if(!remoteRoot.isEmpty() && !remoteRoot.endsWith("/")){
            remoteRoot += "/";
        }
        for(auto file:d->files){
            if(!file.startsWith(projectPath)){
                continue;//not belong to the project
            }
            const QString relative = file.mid(projectPath.length());
            if(relative.isEmpty()){
                continue;
            }
            const QString mapped = req->matchToPath(relative,true,true);
            if(mapped.isEmpty()){
                continue;
            }
            const QString remote = remoteRoot + mapped;
            auto response = req->del(remote);
            if(instance!=nullptr){
                //void* avoids unregistered metatype in queued cross-thread call
                QMetaObject::invokeMethod(instance,"onOutput", Qt::AutoConnection,Q_ARG(void*,response));
            }else{
                delete response;
            }
        }
        return true;
    }
    return false;
}


}

