#include "version_control_delete_file_task.h"
#include "network/network_manager.h"
#include "version_control_pane.h"


namespace ady{

class VersionControlDeleteFileTaskPrivate{
public:
    long long id;
    QStringList files;
};



VersionControlDeleteFileTask::VersionControlDeleteFileTask(long long id,const QStringList& files)
    :BackendThreadTask(BackendThreadTask::Custome)
{
    d = new VersionControlDeleteFileTaskPrivate;
    d->id = id;
    d->files = files;
}


VersionControlDeleteFileTask::~VersionControlDeleteFileTask() {

    delete d;
}


bool VersionControlDeleteFileTask::exec() {
    auto req = NetworkManager::getInstance()->request(d->id);
    if(req!=nullptr){
        auto instance = VersionControlPane::getInstance();
        for(auto file:d->files){
            const QString remote = req->matchToPath(file,true);
            if(!remote.isEmpty()){
                auto response = req->del(remote);
                QMetaObject::invokeMethod(instance,"onOutput", Qt::AutoConnection,Q_ARG(NetworkResponse*,response));
            }
        }
        return true;
    }
    return false;

}


}

