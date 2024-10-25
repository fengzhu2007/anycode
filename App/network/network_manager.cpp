#include "network_manager.h"
#include "transfer/Task.h"
#include "network_response.h"
#include "addon_loader.h"
#include "storage/addon_storage.h"
#include "storage/site_storage.h"
namespace ady {
    NetworkManager* NetworkManager::instance = nullptr;

class NetworkManagerPrivate{
public:
    QMap<QString,QString> addons;
};


    NetworkManager::NetworkManager()
    {
        d = new NetworkManagerPrivate;
        curl_global_init(CURL_GLOBAL_DEFAULT);

    }

    NetworkManager::~NetworkManager(){
        delete d;
    }

    NetworkManager* NetworkManager::getInstance()
    {
        if(instance==nullptr){
            instance = new NetworkManager;
        }
        return instance;
    }

    void NetworkManager::destory(){
        if(instance!=nullptr){
            instance->clear();
            delete instance;
            instance = nullptr;
        }
    }

    NetworkRequest* NetworkManager::request(long long id)
    {
        if(this->requests.contains(id)){
            return this->requests[id];
        }else{
            return nullptr;
        }
    }

    NetworkRequest* NetworkManager::initRequest(long long id,const QString& type){
        if(this->contains(id)){
            return this->requests[id];
        }
        auto loader = AddonLoader::getInstance();
        AddonStorage addonStorage;
        bool ret = false;
        if(d->addons.contains(type)){
            if(d->addons[type].isEmpty()==false){
                ret = loader->load(d->addons[type]);
            }else{
                ret = false;
            }
        }else{
            AddonRecord record = addonStorage.one(type);
            if(record.id>0){
                ret = loader->load(record.file);
                d->addons.insert(record.name,record.file);
            }else{
                d->addons.insert(type,"");
            }
        }
        if(ret){

            auto req = loader->initRequest(id);
            //set password and host
            if(id>0){
                SiteStorage siteStorage;
                SiteRecord record = siteStorage.one(id);
                if(record.id>0){
                    /*req->setHost(record.host);
                    req->setPort(record.port);
                    req->setUsername(record.username);
                    req->setPassword(record.password);*/
                    req->init(record);
                }
            }
            return req;
        }else{
            return nullptr;
        }
    }

    bool NetworkManager::contains(long long id){
        return this->requests.contains(id);
    }

    void NetworkManager::clear()
    {
        QMap<long long ,NetworkRequest*>::iterator iter = this->requests.begin();
        while(iter!=this->requests.end()){
            auto req = iter.value();
            req->unlink();
            iter++;
        }
        this->requests.clear();
    }

    void NetworkManager::remove(long long id)
    {
        if(this->requests.contains(id)){
            auto request = this->requests[id];
            this->requests.erase(this->requests.find(id));//remove from the managers
            request->unlink();
        }
    }

    int NetworkManager::exec(Task* task)
    {
        auto request = this->request(task->siteid);
        NetworkResponse* response = nullptr;
        if(request!=nullptr){
            if(request->isConnected()==false){
                response = request->link();
                int code = 0;
                if(!response->status()){
                    code = -3;
                    task->errorMsg = response->errorMsg;
                }
                delete response;
                response = nullptr;
                if(code!=0){
                    return code;
                }
            }
            if(task->cmd==0){
                //upload
                response = request->upload(task);
            }else if(task->cmd==1){
                //download
                response = request->download(task);
            }else if(task->cmd==2){
                //delete
                response = request->del(task);
            }else if(task->cmd==3){
                //chmod
                response = request->chmod(task);
            }

            if(response==nullptr){
                task->errorMsg = QObject::tr("Invalid task command type:%1").arg(task->cmd);
                return -1;
            }else{
                int code = 0;
                response->debug();
                if(!response->status()){
                    code = response->errorCode==0?response->networkErrorCode:response->errorCode;
                    task->errorMsg = response->errorMsg.isEmpty()?response->networkErrorMsg:response->errorMsg;
                }
                delete response;
                return code;
            }
        }else{
            task->errorMsg = QObject::tr("Invalid site id:%1").arg(task->siteid);
            return -2;
        }
    }


}
