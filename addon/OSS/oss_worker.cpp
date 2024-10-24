#include "oss_worker.h"
#include "oss_response.h"
#include "oss.h"
#include "network/network_manager.h"
#include <QDebug>
namespace ady {

    OSSWorkerTask::OSSWorkerTask(){
        //this->data = nullptr;
    }

    OSSWorkerTask::OSSWorkerTask(QString cmd,QVariant data)
    {
        this->cmd = cmd;
        this->data = data;
    }

    OSSWorkerTask::~OSSWorkerTask()
    {

    }

    constexpr const  char OSSWorker::W_LINK[];
    constexpr const  char OSSWorker::W_CHDIR[] ;
    constexpr const  char OSSWorker::W_UNLINK[];
    constexpr const  char OSSWorker::W_PASV[] ;
    constexpr const  char OSSWorker::W_MKDIR[] ;
    constexpr const  char OSSWorker::W_RENAME[] ;
    constexpr const  char OSSWorker::W_DELETE[] ;
    constexpr const  char OSSWorker::W_ERROR[] ;


    OSSWorker::OSSWorker(long long siteid,QObject* parent)
        :QThread(parent)
    {
        m_siteid = siteid;
        m_mutex = new QMutex;
        m_cond = new QWaitCondition;
        m_is_starting = true;
    }

     OSSWorker::~OSSWorker()
    {
        delete m_cond;
         delete m_mutex;
    }

     OSSWorkerTask* OSSWorker::take()
     {
        QMutexLocker locker(m_mutex);
        while(m_is_starting && m_tasks.size()==0){
            m_cond->wait(m_mutex,5*1000);
        }
        if(!m_is_starting){
            return nullptr;
        }
        return m_tasks.takeFirst();
     }

    void OSSWorker::run(){
        while(true){
            OSSWorkerTask* task = this->take();
            if(task==nullptr){
                break;
            }
            /*OSS* oss = dynamic_cast<OSS*>(NetworkManager::getInstance()->request(m_siteid));
            if(oss!=nullptr){
                //qDebug()<<"OSSWorkerTask tid:"<<QThread::currentThreadId();
                if(task->cmd==W_LINK){
                    emit taskFinished(W_LINK,oss->link());
                }else if(task->cmd==W_CHDIR){
                    emit taskFinished(W_CHDIR,oss->listDir(task->data.toString()));
                }else if(task->cmd==W_UNLINK){
                    emit taskFinished(W_UNLINK,oss->unlink());
                }else if(task->cmd==W_RENAME){
                    QMap<QString,QVariant> data = task->data.toMap();
                    QString src = data["source"].toString();
                    QString dst = data["dst"].toString();
                    emit taskFinished(W_RENAME,oss->rename(src,dst));
                }else if(task->cmd==W_DELETE){
                    DeleteTaskData* data = (DeleteTaskData*)task->data.toLongLong();
                    long long nid = data->nid;
                    int fileCount = 0;
                    int folderCount = 0;
                    QString bucket = "";
                    for(auto item:data->lists){
                        this->del(oss,bucket,item.path,item.type == FileItem::File?0:1,fileCount,folderCount);
                    }
                    delete data;
                    NetworkResponse* response = new OSSResponse;
                    response->errorCode=0;
                    response->params.insert("fileCount",fileCount);
                    response->params.insert("folderCount",folderCount);
                    response->params.insert("nid",nid);
                    emit taskFinished(W_DELETE,response);

                }else if(task->cmd == W_MKDIR){
                    QString filename = task->data.toString();
                    emit taskFinished(W_MKDIR,oss->mkDir(filename));
                }
            }else{
                OSSResponse* response = new OSSResponse();
                response->errorCode = -1;
                response->errorMsg = tr("Invalid OSS connection");
                emit taskFinished(W_ERROR,response);
            }*/
            delete task;
        }
    }

    void OSSWorker::append(OSSWorkerTask* task)
    {
        QMutexLocker locker(m_mutex);
        m_tasks.append(task);
        m_cond->wakeAll();
    }

    void OSSWorker::append(QString cmd,QVariant data)
    {
        OSSWorkerTask* task = new OSSWorkerTask(cmd,data);
        this->append(task);
    }

    void OSSWorker::stop()
    {
        m_is_starting = false;
        m_cond->wakeAll();
    }


    void OSSWorker::del(OSS*oss,QString bucket,QString dst,int type,int& fileCount,int&folderCount)
    {
        if(m_is_starting){
            if(type==1){
                OSSResponse* response = (OSSResponse*)oss->tinyListDir(dst);
                QList<FileItem> lists = response->parseList();
                for(auto item : lists)
                {
                    this->del(oss,bucket,item.path,item.type==FileItem::File?0:1,fileCount,folderCount);
                }
                if(m_is_starting){

                    NetworkResponse* response = oss->rmDir(dst);
                    if(response->status()){
                        folderCount += 1;
                    }
                    delete response;
                }
            }else{
                NetworkResponse* response = oss->del(bucket,dst);
                if(response->status()){
                    fileCount+=1;
                }
                response->debug();
                delete response;
            }
        }
    }





}
