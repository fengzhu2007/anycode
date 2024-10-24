#include "cos_worker.h"
#include "cos_response.h"
#include "cos.h"
#include "network/network_manager.h"
#include <QDebug>
namespace ady {

    COSWorkerTask::COSWorkerTask(){
        //this->data = nullptr;
    }

    COSWorkerTask::COSWorkerTask(QString cmd,QVariant data)
    {
        this->cmd = cmd;
        this->data = data;
    }

    COSWorkerTask::~COSWorkerTask()
    {

    }


    constexpr const  char COSWorker::W_LINK[] ;
    constexpr const  char COSWorker::W_CHDIR[] ;
    constexpr const  char COSWorker::W_UNLINK[];
    constexpr const  char COSWorker::W_MKDIR[] ;
    //constexpr const  char COSWorker::W_RENAME[];
    constexpr const  char COSWorker::W_DELETE[] ;
    constexpr const  char COSWorker::W_ERROR[] ;

    COSWorker::COSWorker(long long siteid,QObject* parent)
        :QThread(parent)
    {
        m_siteid = siteid;
        m_mutex = new QMutex;
        m_cond = new QWaitCondition;
        m_is_starting = true;
    }

     COSWorker::~COSWorker()
    {
        delete m_cond;
         delete m_mutex;
    }

     COSWorkerTask* COSWorker::take()
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

    void COSWorker::run(){
        while(true){
            COSWorkerTask* task = this->take();
            if(task==nullptr){
                break;
            }
            /*COS* cos = dynamic_cast<COS*>(NetworkManager::getInstance()->request(m_siteid));
            if(cos!=nullptr){
                if(task->cmd==W_LINK){
                    emit taskFinished(W_LINK,cos->link());
                }else if(task->cmd==W_CHDIR){
                    emit taskFinished(W_CHDIR,cos->listDir(task->data.toString()));
                }else if(task->cmd==W_UNLINK){
                    emit taskFinished(W_UNLINK,cos->unlink());
                }else if(task->cmd==W_DELETE){
                    DeleteTaskData* data = (DeleteTaskData*)task->data.toLongLong();
                    long long nid = data->nid;
                    int fileCount = 0;
                    int folderCount = 0;
                    QString bucket = "";
                    for(auto item:data->lists){
                        this->del(cos,bucket,item.path,item.type == FileItem::File?0:1,fileCount,folderCount);
                    }
                    delete data;
                    NetworkResponse* response = new COSResponse;
                    response->errorCode=0;
                    response->params.insert("fileCount",fileCount);
                    response->params.insert("folderCount",folderCount);
                    response->params.insert("nid",nid);
                    emit taskFinished(W_DELETE,response);

                }else if(task->cmd == W_MKDIR){
                    QString filename = task->data.toString();
                    emit taskFinished(W_MKDIR,cos->mkDir(filename));
                }
            }else{
                COSResponse* response = new COSResponse();
                response->errorCode = -1;
                response->errorMsg = tr("Invalid COS connection");
                emit taskFinished(W_ERROR,response);
            }*/
            delete task;
        }
    }

    void COSWorker::append(COSWorkerTask* task)
    {
        QMutexLocker locker(m_mutex);
        m_tasks.append(task);
        m_cond->wakeAll();
    }

    void COSWorker::append(QString cmd,QVariant data)
    {
        COSWorkerTask* task = new COSWorkerTask(cmd,data);
        this->append(task);
    }

    void COSWorker::stop()
    {
        m_is_starting = false;
        m_cond->wakeAll();
    }


    void COSWorker::del(COS*cos,QString bucket,QString dst,int type,int& fileCount,int&folderCount)
    {
        if(m_is_starting){
            if(type==1){
                COSResponse* response = (COSResponse*)cos->tinyListDir(dst);
                QList<FileItem> lists = response->parseList();
                for(auto item : lists)
                {
                    this->del(cos,bucket,item.path,item.type==FileItem::File?0:1,fileCount,folderCount);
                }
                if(m_is_starting){

                    NetworkResponse* response = cos->rmDir(dst);
                    if(response->status()){
                        folderCount += 1;
                    }
                    delete response;
                }
            }else{
                NetworkResponse* response = cos->del(bucket,dst);
                if(response->status()){
                    fileCount+=1;
                }
                response->debug();
                delete response;
            }
        }
    }





}
