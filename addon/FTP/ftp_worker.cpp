#include "ftp_worker.h"
#include "ftp_response.h"
#include "ftp.h"
#include "network/network_manager.h"
#include <QDebug>
namespace ady {

    FTPWorkerTask::FTPWorkerTask(){
        //this->data = nullptr;
    }

    FTPWorkerTask::FTPWorkerTask(QString cmd,QVariant data)
    {
        this->cmd = cmd;
        this->data = data;
    }

    FTPWorkerTask::~FTPWorkerTask()
    {

    }


    constexpr const  char FTPWorker::W_LINK[] ;
    constexpr const  char FTPWorker::W_CHDIR[] ;
    constexpr const  char FTPWorker::W_UNLINK[];
    constexpr const  char FTPWorker::W_PASV[] ;
    constexpr const  char FTPWorker::W_MKDIR[] ;
    constexpr const  char FTPWorker::W_RENAME[];
    constexpr const  char FTPWorker::W_CHMOD[] ;
    constexpr const  char FTPWorker::W_DELETE[] ;
    constexpr const  char FTPWorker::W_ERROR[] ;

    FTPWorker::FTPWorker(long long siteid,QObject* parent)
        :QThread(parent)
    {
        m_siteid = siteid;
        m_mutex = new QMutex;
        m_cond = new QWaitCondition;
        m_is_starting = true;
    }

     FTPWorker::~FTPWorker()
    {
        delete m_cond;
         delete m_mutex;
    }

     FTPWorkerTask* FTPWorker::take()
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

    void FTPWorker::run(){
        while(true){
            FTPWorkerTask* task = this->take();
            if(task==nullptr){
                break;
            }
            /*FTP* ftp = dynamic_cast<FTP*>(NetworkManager::getInstance()->request(m_siteid));
            if(ftp!=nullptr){
                //qDebug()<<"FTPWorkerTask tid:"<<QThread::currentThreadId();
                if(task->cmd==W_LINK){
                    emit taskFinished(W_LINK,ftp->link());
                }else if(task->cmd==W_CHDIR){
                    emit taskFinished(W_CHDIR,ftp->listDir(task->data.toString()));
                }else if(task->cmd==W_UNLINK){
                    emit taskFinished(W_UNLINK,ftp->unlink());
                }else if(task->cmd==W_PASV){
                    emit taskFinished(W_PASV,ftp->setPassive(task->data.toBool()));
                }else if(task->cmd==W_RENAME){
                    QMap<QString,QVariant> data = task->data.toMap();
                    QString src = data["source"].toString();
                    QString dst = data["dst"].toString();
                    emit taskFinished(W_RENAME,ftp->rename(src,dst));
                }else if(task->cmd==W_CHMOD){
                    ChmodTaskData* data = (ChmodTaskData*)task->data.toLongLong();
                    long long nid = data->nid;
                    int fileCount = 0;
                    for(auto item:data->lists){
                        this->chmod(ftp,item.path,data->mode,item.type == FileItem::File?0:1,data->applyChildren,fileCount);
                    }
                    delete  data;

                    NetworkResponse* response = new FTPResponse;
                    response->errorCode=0;
                    response->params.insert("fileCount",fileCount);
                    response->params.insert("nid",nid);
                    emit taskFinished(W_CHMOD,response);

                }else if(task->cmd==W_DELETE){
                    DeleteTaskData* data = (DeleteTaskData*)task->data.toLongLong();
                    long long nid = data->nid;
                    int fileCount = 0;
                    int folderCount = 0;
                    for(auto item:data->lists){
                        this->del(ftp,item.path,item.type == FileItem::File?0:1,fileCount,folderCount);
                    }
                    delete data;
                    NetworkResponse* response = new FTPResponse;
                    response->errorCode=0;
                    response->params.insert("fileCount",fileCount);
                    response->params.insert("folderCount",folderCount);
                    response->params.insert("nid",nid);
                    emit taskFinished(W_DELETE,response);

                }else if(task->cmd == W_MKDIR){
                    QString filename = task->data.toString();
                    emit taskFinished(W_MKDIR,ftp->mkDir(filename));
                }
            }else{
                FTPResponse* response = new FTPResponse();
                response->errorCode = -1;
                response->errorMsg = tr("Invalid FTP connection");
                emit taskFinished(W_ERROR,response);
            }*/
            delete task;
        }
    }

    void FTPWorker::append(FTPWorkerTask* task)
    {
        QMutexLocker locker(m_mutex);
        m_tasks.append(task);
        m_cond->wakeAll();
    }

    void FTPWorker::append(QString cmd,QVariant data)
    {
        FTPWorkerTask* task = new FTPWorkerTask(cmd,data);
        this->append(task);
    }

    void FTPWorker::stop()
    {
        m_is_starting = false;
        m_cond->wakeAll();
    }


    void FTPWorker::chmod(FTP*ftp,QString dst,int mode,int type,bool applyChildren,int& fileCont)
    {
        if(m_is_starting){
            NetworkResponse* response = ftp->chmod(dst,mode);
            if(response->status()){
                fileCont += 1;
            }
            response->debug();
            delete response;
            if(m_is_starting && type==1 && applyChildren){
                FTPResponse* response = (FTPResponse*)ftp->tinyListDir(dst);
                QList<FileItem> lists = response->parseList();
                for(auto item : lists)
                {
                    this->chmod(ftp,item.path,mode,item.type==FileItem::File?0:1,applyChildren,fileCont);
                }
            }
        }
    }

    void FTPWorker::del(FTP*ftp,QString dst,int type,int& fileCount,int&folderCount)
    {
        if(m_is_starting){
            if(type==1){
                FTPResponse* response = (FTPResponse*)ftp->tinyListDir(dst);
                QList<FileItem> lists = response->parseList();
                for(auto item : lists)
                {
                    this->del(ftp,item.path,item.type==FileItem::File?0:1,fileCount,folderCount);
                }
                if(m_is_starting){

                    NetworkResponse* response = ftp->rmDir(dst);
                    if(response->status()){
                        folderCount += 1;
                    }
                    delete response;
                }
            }else{
                NetworkResponse* response = ftp->del(dst);
                if(response->status()){
                    fileCount+=1;
                }
                response->debug();
                delete response;
            }
        }
    }





}
