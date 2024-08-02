#include "TaskThread.h"
#include "TaskPoolModel.h"
#include "Task.h"
#include "network/network_manager.h"
#include "network/network_response.h"
#include "transfer/TaskPoolModel.h"
#include "local/file_item.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
namespace ady {
    TaskThread::TaskThread(long long siteid,QObject* parent)
        :QThread(parent)
    {
        m_siteid = siteid;
    }

    void TaskThread::run()
    {
        while(true){

            Task* task = TaskPoolModel::getInstance()->take(m_siteid);

            //qDebug()<<"task:"<<task;
            /*if(task==nullptr){
                task = TaskPoolModel::getInstance()->take();
            }*/
            if(task==nullptr){
                continue;
            }
            //qDebug()<<"task site:"<<task->siteid<<";thread site:"<<m_siteid<<";file:"<<task->local;
            //long long siteid = task->siteid;
            //this->type = 0;//0=file 1=dir
            if(task->type==0){
                //file
                int ret = NetworkManager::getInstance()->exec(task);
                //QThread::sleep(2);
                //int ret = 0;
                if(task->abort==true){
                    delete task;
                    continue;
                }
                if(ret==0){
                    //TaskPoolModel::getInstance()->removeTask(task);
                    emit removeTask(task);
                }else{
                    //TaskPoolModel::getInstance()->failTask(task);
                    emit failTask(task);
                }
            }else if(task->type==1){
                //dir
                QList<Task*> tasks ;
                if(task->cmd==0){
                    //upload traval local dir
                    tasks = this->travelLocalDir(task);
                }else{
                    if(task->cmd==1){
                        //download
                        QString local = task->local;
                        if(local.endsWith("/")){
                            local = local.left(local.size() - 1);
                        }
                        QFileInfo fi (local);
                        if(fi.exists()==false){
                            QDir d = fi.dir();
                            if(!d.mkdir(fi.fileName())){
                                //mkdir failed
                                task->errorMsg = tr("Folder creation failed");
                                //TaskPoolModel::getInstance()->failTask(task);
                                emit failTask(task);
                                continue;
                            }
                        }
                    }
                    //download delete chmod remote dir
                    if(task->cmd==1 /*|| task->cmd==2 || (task->cmd==3 && task->data.contains(Task::APPLY_CHILDREN) && task->data[Task::APPLY_CHILDREN].toBool())*/){
                        //chmode
                        tasks = this->travelRemoteDir(task);
                    }
                }

                if(task->abort==true){
                    delete task;
                    QList<Task*>::iterator iter = tasks.begin();
                    while(iter!=tasks.end()){
                        delete *iter;
                        iter++;
                    }
                    tasks.clear();
                    continue;
                }
                emit removeTask(task);
                //qDebug()<<"insert children tasks:"<<tasks.size();
                if(tasks.size()>0){
                    emit insertTasks(tasks);
                }

            }
        }
    }


    QList<Task*> TaskThread::travelLocalDir(Task* task){
        QDir d(task->local);
        QFileInfoList lists = d.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name|QDir::DirsFirst);
        int length = lists.length();
        QList<Task*> tasks;
        for(int i=0;i<length;i++){
            QFileInfo fi = lists.at(i);
            Task* t = new Task;
            t->local = fi.absoluteFilePath();
            t->remote = task->remote + "/" + fi.fileName();
            //qDebug()<<"local:"<<t->local<<";remote:"<<t->remote;
            t->cmd = task->cmd;
            t->type = fi.isFile()?0:1;
            t->siteid = task->siteid;
            if(t->type==0){
                t->filesize = fi.size();
            }
            t->data = task->data;
            tasks.push_back(t);
        }
        return tasks;
    }

    QList<Task*> TaskThread::travelRemoteDir(Task* task)
    {
        QList<Task*> tasks;
        auto request = NetworkManager::getInstance()->request(task->siteid);
        if(request!=nullptr){
            qDebug()<<"request result";
            NetworkResponse* response = request->tinyListDir(task->remote);
            response->debug();
            if(response->errorCode==0){
                //ok
                QList<FileItem> items = response->parseList();
                QList<FileItem>::iterator iter = items.begin();
                QString local = task->local;
                if(!local.endsWith("/")){
                    local += "/";
                }
                while(iter!=items.end()){
                    Task* t = new Task;

                    t->local = local + (*iter).name;
                    t->remote = (*iter).path;
                    //qDebug()<<"local:"<<t->local<<";remote:"<<t->remote;
                    t->cmd = task->cmd;
                    t->type = (*iter).type==FileItem::File?0:1;
                    t->siteid = task->siteid;
                    if(t->type==0){
                        t->filesize = (*iter).size;
                    }
                    t->data = task->data;
                    tasks.push_back(t);
                    iter++;
                }
            }
        }
        return tasks;
    }


}
