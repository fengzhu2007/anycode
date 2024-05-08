#include "SFTPWorker.h"
#include "SFTPResponse.h"
#include "sftp.h"
#include "network/NetworkManager.h"
#include <QDebug>
namespace ady {

    SFTPWorkerTask::SFTPWorkerTask(){
        //this->data = nullptr;
    }

    SFTPWorkerTask::SFTPWorkerTask(QString cmd,QVariant data)
    {
        this->cmd = cmd;
        this->data = data;
    }

    SFTPWorkerTask::~SFTPWorkerTask()
    {

    }


    constexpr const  char SFTPWorker::W_LINK[] ;
    constexpr const  char SFTPWorker::W_CHDIR[] ;
    constexpr const  char SFTPWorker::W_UNLINK[];
    //constexpr const  char SFTPWorker::W_PASV[] ;
    constexpr const  char SFTPWorker::W_MKDIR[] ;
    constexpr const  char SFTPWorker::W_RENAME[];
    constexpr const  char SFTPWorker::W_CHMOD[] ;
    constexpr const  char SFTPWorker::W_DELETE[] ;
    constexpr const  char SFTPWorker::W_ERROR[] ;

    SFTPWorker::SFTPWorker(long long siteid,QObject* parent)
        :QThread(parent)
    {
        m_siteid = siteid;
        m_mutex = new QMutex;
        m_cond = new QWaitCondition;
        m_is_starting = true;
    }

     SFTPWorker::~SFTPWorker()
    {
        delete m_cond;
         delete m_mutex;
    }

     SFTPWorkerTask* SFTPWorker::take()
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

    void SFTPWorker::run(){
        while(true){
            SFTPWorkerTask* task = this->take();
            if(task==nullptr){
                break;
            }
            SFTP* sftp = dynamic_cast<SFTP*>(NetworkManager::getInstance()->request(m_siteid));
            if(sftp!=nullptr){
                //qDebug()<<"SFTPWorkerTask tid:"<<QThread::currentThreadId();
                if(task->cmd==W_LINK){
                    emit taskFinished(W_LINK,sftp->link());
                }else if(task->cmd==W_CHDIR){
                    emit taskFinished(W_CHDIR,sftp->listDir(task->data.toString()));
                }else if(task->cmd==W_UNLINK){
                    emit taskFinished(W_UNLINK,sftp->unlink());
                }/*else if(task->cmd==W_PASV){
                    emit taskFinished(W_PASV,sftp->setPassive(task->data.toBool()));
                }*/else if(task->cmd==W_RENAME){
                    QMap<QString,QVariant> data = task->data.toMap();
                    QString src = data["source"].toString();
                    QString dst = data["dst"].toString();
                    emit taskFinished(W_RENAME,sftp->rename(src,dst));
                }else if(task->cmd==W_CHMOD){
                    ChmodTaskData* data = (ChmodTaskData*)task->data.toLongLong();
                    long long nid = data->nid;
                    int fileCount = 0;
                    for(auto item:data->lists){
                        this->chmod(sftp,item.path,data->mode,item.type == FileItem::File?0:1,data->applyChildren,fileCount);
                    }
                    delete  data;

                    NetworkResponse* response = new SFTPResponse;
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
                        this->del(sftp,item.path,item.type == FileItem::File?0:1,fileCount,folderCount);
                    }
                    delete data;
                    NetworkResponse* response = new SFTPResponse;
                    response->errorCode=0;
                    response->params.insert("fileCount",fileCount);
                    response->params.insert("folderCount",folderCount);
                    response->params.insert("nid",nid);
                    emit taskFinished(W_DELETE,response);

                }else if(task->cmd == W_MKDIR){
                    QString filename = task->data.toString();
                    emit taskFinished(W_MKDIR,sftp->mkDir(filename));
                }
            }else{
                SFTPResponse* response = new SFTPResponse();
                response->errorCode = -1;
                response->errorMsg = tr("Invalid SFTP connection");
                emit taskFinished(W_ERROR,response);
            }
            delete task;
        }
    }

    void SFTPWorker::append(SFTPWorkerTask* task)
    {
        QMutexLocker locker(m_mutex);
        m_tasks.append(task);
        m_cond->wakeAll();
    }

    void SFTPWorker::append(QString cmd,QVariant data)
    {
        SFTPWorkerTask* task = new SFTPWorkerTask(cmd,data);
        this->append(task);
    }

    void SFTPWorker::stop()
    {
        m_is_starting = false;
        m_cond->wakeAll();
    }


    void SFTPWorker::chmod(SFTP*sftp,QString dst,int mode,int type,bool applyChildren,int& fileCont)
    {
        if(m_is_starting){
            NetworkResponse* response = sftp->chmod(dst,mode);
            if(response->status()){
                fileCont += 1;
            }
            response->debug();
            delete response;
            if(m_is_starting && type==1 && applyChildren){
                SFTPResponse* response = (SFTPResponse*)sftp->tinyListDir(dst);
                QList<FileItem> lists = response->parseList();
                for(auto item : lists)
                {
                    this->chmod(sftp,item.path,mode,item.type==FileItem::File?0:1,applyChildren,fileCont);
                }
            }
        }
    }

    void SFTPWorker::del(SFTP*sftp,QString dst,int type,int& fileCount,int&folderCount)
    {
        if(m_is_starting){
            if(type==1){
                SFTPResponse* response = (SFTPResponse*)sftp->tinyListDir(dst);
                QList<FileItem> lists = response->parseList();
                for(auto item : lists)
                {
                    this->del(sftp,item.path,item.type==FileItem::File?0:1,fileCount,folderCount);
                }
                if(m_is_starting){

                    NetworkResponse* response = sftp->rmDir(dst);
                    if(response->status()){
                        folderCount += 1;
                    }
                    delete response;
                }
            }else{
                NetworkResponse* response = sftp->del(dst);
                if(response->status()){
                    fileCount+=1;
                }
                response->debug();
                delete response;
            }
        }
    }





}
