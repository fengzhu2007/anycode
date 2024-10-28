#include "server_request_thread.h"
#include "server_manage_model.h"
#include <QDebug>
namespace ady{
class ServerRequestThreadPrivate{
public:
    NetworkRequest* req;
    int cmd;
    void* data;
    bool interrupt = false;
};

ServerRequestThread::ServerRequestThread(NetworkRequest* request,int command,void* data,QObject *parent)
    : QThread{parent}
{
    d = new ServerRequestThreadPrivate;
    d->req = request;
    d->cmd = command;
    d->data = data;
}

ServerRequestThread::~ServerRequestThread(){
    delete d;
}

void ServerRequestThread::interrupt(){
    d->interrupt = true;
}

void ServerRequestThread::run(){
    NetworkResponse* response = nullptr;
    int result = 0;
    if(d->cmd==Link){
        response = d->req->link();
        if(response->status()){
            //cd data
            delete response;
            response = nullptr;
            QString* path = static_cast<QString*>(d->data);
            response = d->req->listDir(*path);
            delete path;
        }
    }else if(d->cmd==List){
        QString* path = static_cast<QString*>(d->data);
        response = d->req->listDir(*path);
        delete path;
    }else if(d->cmd==Refresh){
        QString* path = static_cast<QString*>(d->data);
        response = d->req->listDir(*path);
        delete path;
    }else if(d->cmd==NewFolder){
        QString* path = static_cast<QString*>(d->data);
        response = d->req->mkDir(*path);
        if(response->status()){
            delete response;
            const QString dir = path->left(path->lastIndexOf("/") + 1);
            response = d->req->listDir(dir);
        }
        delete path;
    }else if(d->cmd==Rename){
        auto data = static_cast<RenameData*>(d->data);
        response = d->req->rename(data->src,data->dest);
        if(response!=nullptr){
             delete response;
        }else{
             result = Unsppport;
        }
        response = d->req->listDir(data->parent);
        delete data;
    }else if(d->cmd==Delete){
        auto list = static_cast<QStringList*>(d->data);
        QString parent;
        int successTotal = 0;
        int errorTotal = 0;
        for(auto one:*list){
             if(response!=nullptr){
                 delete response;
             }
             QString path;
             if(one.endsWith("/")){
                 //folder
                 //read all sub files

                 this->delFolder(one,&successTotal,&errorTotal);
                 path = one.left(one.size() - 1);
             }else{
                //file
                 //response = d->req->del(one);
                 bool ret = this->delFile(one);
                 if(ret){
                     successTotal+=1;
                 }else{
                     errorTotal+=1;
                 }
                 path = one;
             }
             path = path.left(path.lastIndexOf("/")+1);
             if(parent.isEmpty() || parent.contains(path)){
                 parent = path;
             }
             if(d->interrupt){
                 goto interrupt;
             }
        }

        if(errorTotal>0){
             emit message(tr("%1 files and directories deleted successfully, %2 files and directories deleted failed.").arg(successTotal).arg(errorTotal),Failed);
        }
        delete list;
        if(!parent.isEmpty()){
            delete response;
            response = d->req->listDir(parent);
        }
        //refresh parent
    }else if(d->cmd==Chmod){

    }else if(d->cmd==Unlink){
        response = d->req->unlink();
    }

interrupt:
    if(d->interrupt){
        result = Interrupt;
    }

    emit resultReady(response,d->cmd,result);
}

bool ServerRequestThread::delFile(const QString& path){
    auto response = d->req->del(path);
    bool ret = false;
    if(response->status()){
        ret = true;
    }
    delete response;
    return ret;
}

void ServerRequestThread::delFolder(const QString& path,int* successTotal,int* errorTotal){
    auto response = d->req->tinyListDir(path);
    auto list = response->parseList();
    delete response;
    for(auto one:list){
        if(d->interrupt){
            return ;
        }
        if(one.type==FileItem::File){
            bool ret = this->delFile(one.path);
            if(ret){
                 *successTotal += 1;
            }else{
                 *errorTotal += 1;
            }
        }else{
            //folder
            this->delFolder(one.path,successTotal,errorTotal);
        }
    }
    response = d->req->rmDir(path);
    if(response->status()){
        *successTotal += 1;
    }else{
        *errorTotal += 1;
    }
    response->debug();
    delete response;
}

}
