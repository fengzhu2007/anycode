#include "job_thread.h"
#include "file_transfer_model.h"
#include "transfer/task.h"
#include "network/network_manager.h"
#include "network/network_response.h"

#include <QDir>
namespace ady{
JobThread::JobThread(long long id,FileTransferModel *parent)
    : QThread(parent),m_siteid(id),m_task(nullptr)
{

}


void JobThread::run(){
    auto model = static_cast<FileTransferModel*>(this->parent());
    while(true){
        auto item = model->take(m_siteid);
        if(this->isInterruptionRequested()){
            break;
        }
        if(item==nullptr){
            continue;
        }
        //Task* task = nullptr;
        if(item->type()==FileTransferModelItem::Job){
            m_task = new Task(item);
            int ret = NetworkManager::getInstance()->exec(m_task);
            if(ret==0){
                //ok
                //model->removeItem(m_task->siteid,m_task->id);//ok remove task
                emit finishTask(m_task->siteid,m_task->id);
            }else{
                //error
                //qDebug()<<"error:"<<m_task->errorMsg<<ret;
                //model->setItemFailed(m_task->siteid,m_task->id,m_task->errorMsg);//ok remove task
                emit errorTask(m_task->siteid,m_task->id,m_task->errorMsg);
            }
            delete m_task;
            m_task = nullptr;
            //model->removeItem(item->parent()->id(),item->id());
        }else{
            //QList<FileTransferModelItem*> list;
            long long siteid = item->parent()->id();
            long long id = item->id();
            const QString source = item->source();
            const QString destination = item->destination();
            if(item->mode()==FileTransferModelItem::Upload){
                //upload folder
                auto list = this->listLocal(source);
                //model->insertFrontAndRemove(siteid,id,list,FileTransferModelItem::Wait);
                emit uploadFolder(siteid,id,list,FileTransferModelItem::Wait);
            }else{
                //download folder
                QString errorMsg;
                auto list = this->listRemote(siteid,destination,&errorMsg);
                if(!errorMsg.isEmpty()){
                    //model->setItemFailed(siteid,id,errorMsg);//ok remove task
                    emit errorTask(siteid,id,errorMsg);
                }else{
                    //model->insertFrontAndRemove(siteid,id,list,FileTransferModelItem::Wait);
                    emit donwloadFolder(siteid,id,list,FileTransferModelItem::Wait);

                    //void finishTask(long long siteid,long long id);
                    //void uploadFolder(long long siteid,long long id,QFileInfoList list,int state);
                    //void donwloadFolder(long long siteid,long long id,QList<FileItem> list,int state);


                }
            }
        }

    }
}


void JobThread::abort(){
    if(m_task!=nullptr){
        m_task->abort = true;
    }
}


QFileInfoList JobThread::listLocal(const QString& source){
    QDir d(source);
    QFileInfoList lists = d.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name|QDir::DirsFirst);
    return lists;
}

QList<FileItem> JobThread::listRemote(long long siteid,const QString& destination,QString* errorMsg)
{
    QList<FileItem> list;
    auto request = NetworkManager::getInstance()->request(siteid);
    if(request!=nullptr){
        NetworkResponse* response = nullptr;
        if(request->isConnected()==false){
            response = request->link();
            bool status = response->status();
            if(!status){
                *errorMsg = response->errorMsg;
            }
            delete response;
            if(status==false){
                if(response->errorMsg.isEmpty()){
                    *errorMsg = tr("Server connect failed");
                }else{
                    *errorMsg = response->errorMsg;
                }
                return list;
            }
        }
        response = request->tinyListDir(destination);
        if(response->errorCode==0){
            //ok
            list = response->parseList();
        }else{
            if(response->errorMsg.isEmpty()){
                *errorMsg = tr("Remote folder access failed");
            }else{
                *errorMsg = response->errorMsg;
            }
        }
        delete response;
    }
    return list;
}

}
