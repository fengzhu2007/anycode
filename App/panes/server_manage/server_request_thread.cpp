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
        for(auto one:*list){
             if(response!=nullptr){
                 delete response;
             }
             QString path;
             if(one.endsWith("/")){
                 //folder
                 response = d->req->rmDir(one);
                 path = one.left(one.size() - 1);
             }else{
                //file
                 response = d->req->del(one);
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

void ServerRequestThread::delFile(const QString& path){
    d->req->del(path);
}

void ServerRequestThread::delFolder(const QString& path){

}

}
