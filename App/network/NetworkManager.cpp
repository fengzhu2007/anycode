#include "NetworkManager.h"
#include "transfer/Task.h"
#include "NetworkResponse.h"
namespace ady {
    NetworkManager* NetworkManager::instance = nullptr;

    NetworkManager::NetworkManager()
    {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    NetworkManager* NetworkManager::getInstance()
    {
        if(instance==nullptr){
            instance = new NetworkManager;
        }
        return instance;
    }

    NetworkRequest* NetworkManager::request(long long id)
    {
        if(this->requests.contains(id)){
            return this->requests[id];
        }else{
            return nullptr;
        }
    }

    void NetworkManager::clear()
    {
        QMap<long long ,NetworkRequest*>::iterator iter = this->requests.begin();
        while(iter!=this->requests.end()){
            NetworkRequest* request = iter.value();
            request->unlink();
            delete request;
            iter++;
        }
        this->requests.clear();
    }

    void NetworkManager::remove(long long id)
    {
        if(this->requests.contains(id)){
            NetworkRequest* request = this->requests[id];
            this->requests.erase(this->requests.find(id));//remove from the managers
            request->unlink();
            delete request;
        }
    }

    int NetworkManager::exec(Task* task)
    {
        NetworkRequest* request = this->request(task->siteid);
        NetworkResponse* response = nullptr;
        if(request!=nullptr){
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
