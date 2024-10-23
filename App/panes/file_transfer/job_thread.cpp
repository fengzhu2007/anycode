#include "job_thread.h"
#include "file_transfer_model.h"
#include "transfer/Task.h"
#include "network/network_manager.h"
#include "network/network_response.h"
#include "local/file_item.h"
#include <QDir>
namespace ady{
JobThread::JobThread(long long id,FileTransferModel *parent)
    : QThread(parent),m_siteid(id)
{

}


void JobThread::run(){
    auto model = static_cast<FileTransferModel*>(this->parent());
    while(true){
        auto item = model->take(m_siteid);
        if(item==nullptr){
            continue;
        }
        Task* task = nullptr;
        if(item->type()==FileTransferModelItem::Job){
            task = new Task(item);
            int ret = NetworkManager::getInstance()->exec(task);




            //model->removeItem(item->parent()->id(),item->id());
        }else{
            QList<FileTransferModelItem*> list;
            if(item->mode()==FileTransferModelItem::Upload){
                //upload folder
                list = this->listLocal(item);
            }else{
                //download folder
                list = this->listRemote(item);
            }



            model->removeItem(item->parent()->id(),item->id());
            model->insertFrontItems(item->parent(),list,FileTransferModelItem::Wait);
        }

    }
}


QList<FileTransferModelItem*> JobThread::listLocal(FileTransferModelItem* parent){
    QDir d(parent->source());
    QFileInfoList lists = d.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name|QDir::DirsFirst);
    int length = lists.length();
    QList<FileTransferModelItem*> items;
    for(int i=0;i<length;i++){
        QFileInfo fi = lists.at(i);
        auto item = new FileTransferModelItem(FileTransferModelItem::Upload,fi.isFile(),fi.absoluteFilePath(),parent->destination()+"/"+fi.fileName(),parent);
        items<<item;
    }
    return items;
}

QList<FileTransferModelItem*> JobThread::listRemote(FileTransferModelItem* parent)
{
    QList<FileTransferModelItem*> items;
    auto request = NetworkManager::getInstance()->request(parent->parent()->id());
    if(request!=nullptr){
        NetworkResponse* response = request->tinyListDir(parent->destination());
        //response->debug();
        if(response->errorCode==0){
            //ok
            QList<FileItem> list = response->parseList();
            //QList<FileItem>::iterator iter = items.begin();
            QString local = parent->source();
            if(!local.endsWith("/")){
                local += "/";
            }
            for(auto one:list){
                 auto item = new FileTransferModelItem(FileTransferModelItem::Download,one.type==FileItem::File,local+one.name,one.path,parent);
                items<<item;
            }
        }
    }
    return items;
}

}
