#include "{TPL}Worker.h"
#include "{TPL}Response.h"
#include "{tpl}.h"
#include "network/NetworkManager.h"
#include <QDebug>
namespace ady {

    {TPL}WorkerTask::{TPL}WorkerTask(){
        //this->data = nullptr;
    }

    {TPL}WorkerTask::{TPL}WorkerTask(QString cmd,QVariant data)
    {
        this->cmd = cmd;
        this->data = data;
    }

    {TPL}WorkerTask::~{TPL}WorkerTask()
    {

    }


    constexpr const  char {TPL}Worker::W_LINK[] ;
    constexpr const  char {TPL}Worker::W_CHDIR[] ;
    constexpr const  char {TPL}Worker::W_UNLINK[];
    constexpr const  char {TPL}Worker::W_PASV[] ;
    constexpr const  char {TPL}Worker::W_MKDIR[] ;
    constexpr const  char {TPL}Worker::W_RENAME[];
    constexpr const  char {TPL}Worker::W_CHMOD[] ;
    constexpr const  char {TPL}Worker::W_DELETE[] ;
    constexpr const  char {TPL}Worker::W_ERROR[] ;

    {TPL}Worker::{TPL}Worker(long long siteid,QObject* parent)
        :QThread(parent)
    {
        m_siteid = siteid;
        m_mutex = new QMutex;
        m_cond = new QWaitCondition;
        m_is_starting = true;
    }

     {TPL}Worker::~{TPL}Worker()
    {
        delete m_cond;
         delete m_mutex;
    }

     {TPL}WorkerTask* {TPL}Worker::take()
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

    void {TPL}Worker::run(){
        while(true){
            {TPL}WorkerTask* task = this->take();
            if(task==nullptr){
                break;
            }
            {TPL}* {tpl} = dynamic_cast<{TPL}*>(NetworkManager::getInstance()->request(m_siteid));
            if({tpl}!=nullptr){
                //qDebug()<<"{TPL}WorkerTask tid:"<<QThread::currentThreadId();
                if(task->cmd==W_LINK){
                    emit taskFinished(W_LINK,{tpl}->link());
                }else if(task->cmd==W_CHDIR){
                    emit taskFinished(W_CHDIR,{tpl}->listDir(task->data.toString()));
                }else if(task->cmd==W_UNLINK){
                    emit taskFinished(W_UNLINK,{tpl}->unlink());
                }else if(task->cmd==W_PASV){
                    emit taskFinished(W_PASV,{tpl}->setPassive(task->data.toBool()));
                }else if(task->cmd==W_RENAME){
                    QMap<QString,QVariant> data = task->data.toMap();
                    QString src = data["source"].toString();
                    QString dst = data["dst"].toString();
                    emit taskFinished(W_RENAME,{tpl}->rename(src,dst));
                }else if(task->cmd==W_CHMOD){
                    ChmodTaskData* data = (ChmodTaskData*)task->data.toLongLong();
                    long long nid = data->nid;
                    int fileCount = 0;
                    for(auto item:data->lists){
                        this->chmod({tpl},item.path,data->mode,item.type == FileItem::File?0:1,data->applyChildren,fileCount);
                    }
                    delete  data;

                    NetworkResponse* response = new {TPL}Response;
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
                        this->del({tpl},item.path,item.type == FileItem::File?0:1,fileCount,folderCount);
                    }
                    delete data;
                    NetworkResponse* response = new {TPL}Response;
                    response->errorCode=0;
                    response->params.insert("fileCount",fileCount);
                    response->params.insert("folderCount",folderCount);
                    response->params.insert("nid",nid);
                    emit taskFinished(W_DELETE,response);

                }else if(task->cmd == W_MKDIR){
                    QString filename = task->data.toString();
                    emit taskFinished(W_MKDIR,{tpl}->mkDir(filename));
                }
            }else{
                {TPL}Response* response = new {TPL}Response();
                response->errorCode = -1;
                response->errorMsg = tr("Invalid {TPL} connection");
                emit taskFinished(W_ERROR,response);
            }
            delete task;
        }
    }

    void {TPL}Worker::append({TPL}WorkerTask* task)
    {
        QMutexLocker locker(m_mutex);
        m_tasks.append(task);
        m_cond->wakeAll();
    }

    void {TPL}Worker::append(QString cmd,QVariant data)
    {
        {TPL}WorkerTask* task = new {TPL}WorkerTask(cmd,data);
        this->append(task);
    }

    void {TPL}Worker::stop()
    {
        m_is_starting = false;
        m_cond->wakeAll();
    }


    void {TPL}Worker::chmod({TPL}*{tpl},QString dst,int mode,int type,bool applyChildren,int& fileCont)
    {
        if(m_is_starting){
            NetworkResponse* response = {tpl}->chmod(dst,mode);
            if(response->status()){
                fileCont += 1;
            }
            response->debug();
            delete response;
            if(m_is_starting && type==1 && applyChildren){
                {TPL}Response* response = ({TPL}Response*){tpl}->tinyListDir(dst);
                QList<FileItem> lists = response->parseList();
                for(auto item : lists)
                {
                    this->chmod({tpl},item.path,mode,item.type==FileItem::File?0:1,applyChildren,fileCont);
                }
            }
        }
    }

    void {TPL}Worker::del({TPL}*{tpl},QString dst,int type,int& fileCount,int&folderCount)
    {
        if(m_is_starting){
            if(type==1){
                {TPL}Response* response = ({TPL}Response*){tpl}->tinyListDir(dst);
                QList<FileItem> lists = response->parseList();
                for(auto item : lists)
                {
                    this->del({tpl},item.path,item.type==FileItem::File?0:1,fileCount,folderCount);
                }
                if(m_is_starting){

                    NetworkResponse* response = {tpl}->rmDir(dst);
                    if(response->status()){
                        folderCount += 1;
                    }
                    delete response;
                }
            }else{
                NetworkResponse* response = {tpl}->del(dst);
                if(response->status()){
                    fileCount+=1;
                }
                response->debug();
                delete response;
            }
        }
    }





}
