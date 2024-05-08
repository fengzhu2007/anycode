#include "{tpl}.h"
#include "{TPL}Response.h"
#include "transfer/Task.h"
#include "utils.h"
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QDebug>
namespace ady {
    {TPL}::{TPL}(CURL* curl)
        :NetworkRequest(curl)
    {
        this->connected = false;
        this->mlsd = false;
        this->mfmt = false;
        this->utf8 = false;
        this->setOption(CURLOPT_TIMEOUT,COMMAND_TIMEOUT);
    }

    int {TPL}::access(NetworkResponse* response,bool body)
    {
        this->setHeaderHolder(&response->header);
        if(body){
            this->setBodyHolder(&response->body);
        }
        CURLcode res = curl_easy_perform(this->curl);
        if(res!=CURLE_OK && res != CURLE_{TPL}_COULDNT_RETR_FILE ){
            this->errorMsg = curl_easy_strerror(res);
            this->errorCode = (int)res;
        }else{
            this->errorMsg = "";
            this->errorCode = 0;
        }
        response->errorCode = this->errorCode;
        response->errorMsg = this->errorMsg;
        response->parse();
        return (int)res;
    }

    NetworkResponse* {TPL}::link()
    {
        QMutexLocker locker(&mutex);
        QString host = "{tpl}://" + this->host;
        host += "/";
        this->setOption(CURLOPT_URL,host.toStdString().c_str());
        this->setOption(CURLOPT_PORT,this->port);
        this->setOption(CURLOPT_TIMEOUT,COMMAND_TIMEOUT);
        this->setOption(CURLOPT_CONNECTTIMEOUT,COMMAND_TIMEOUT);
        this->setOption(CURLOPT_USERNAME,this->username.toStdString().c_str());
        this->setOption(CURLOPT_PASSWORD,this->password.toStdString().c_str());
        NetworkResponse* response = new {TPL}Response;
        int ret = this->access(response);
        if(ret==0){
            this->connected = true;
            this->initEnv();
        }
        //delete response;
        return response;
    }

    NetworkResponse* {TPL}::unlink()
    {
        QMutexLocker locker(&mutex);
        this->setOption(CURLOPT_TIMEOUT,UNLINK_TIMEOUT);
        QString command = "QUIT";
        NetworkResponse* response = this->sendCommand(command);
        this->setOption(CURLOPT_TIMEOUT,COMMAND_TIMEOUT);
        return response;
    }


    NetworkResponse* {TPL}::sendSyncCommand(QString command)
    {
        QMutexLocker locker(&mutex);
        return this->sendCommand(command);
    }

    void {TPL}::initEnv()
    {
        NetworkResponse* response = this->sendCommand("FEAT");
        int ret = response->errorCode;

        if(ret == 0){
            //QStringList lines = response->header.split("\r\n");

            //delete response;
            //response->debug();
            int size = response->headers.size();
            bool start211 = false;
            for(int i=0;i<size;i++){
                if(response->headers[i].left(3)=="211"&&start211==true){
                    start211 = false;
                }
                if(start211==true){

                    QString name = response->headers[i].trimmed();
                    if(name=="MLSD"){
                        this->mlsd = true;
                    }else if(name=="MFMT"){
                        this->mfmt = true;
                    }else if(name=="UTF8"){
                        this->utf8 = true;
                    }
                    this->feats.push_back(name);
                }
                //qDebug()<<"feats:"<<this->feats;
                if(response->headers[i].left(3)=="211"&&start211==false){
                    start211 = true;
                }
            }
            delete response;

            if(this->utf8){
                NetworkResponse* response = this->sendCommand("OPTS UTF8 ON");
                delete response;
            }
        }
    }


    NetworkResponse* {TPL}::sendCommand(QString command)
    {
        //qDebug()<<"exec command:"<<command;
        this->setOption(CURLOPT_USERNAME,this->username.toStdString().c_str());
        this->setOption(CURLOPT_PASSWORD,this->password.toStdString().c_str());
        this->setOption(CURLOPT_CUSTOMREQUEST,command.toStdString().c_str());
        {TPL}Response* response = new {TPL}Response;
        response->setCommand(command);
        this->errorCode = this->access(response);
        this->header = response->header;;
        this->body = response->body;
        response->parse();
        return response;
    }

    NetworkResponse* {TPL}::listDir(QString dir,int page,int pageSize)
    {
        Q_UNUSED(page);
        Q_UNUSED(pageSize);

        QMutexLocker locker(&mutex);
        QString command = "CWD "+dir;
        NetworkResponse* response = this->sendCommand(command);
        if(response->errorCode==0){
            delete response;
             command = "LIST -al";
             if(this->mlsd){
                command = "MLSD";
             }
             response = this->sendCommand(command);
             if(response->errorCode==0){
                 response->params["dir"] = dir;
             }
        }
        return response;

    }

    NetworkResponse* {TPL}::tinyListDir(QString dir)
    {
        QMutexLocker locker(&mutex);
        QString command = "LIST "+dir/*+" -al"*/;//windows error
        NetworkResponse* response =  this->sendCommand(command);
        if(response->errorCode==0){
            response->params["dir"] = dir;
        }
        qDebug()<<"111111111111111111111111111111";
        response->debug();
        return response;
    }

    NetworkResponse* {TPL}::upload(Task* task)
    {
        QMutexLocker locker(&mutex);
        QString local = task->local;
        QString remote = task->remote;
        NetworkResponse *response =  new {TPL}Response;
        QFileInfo fi(local);
        if(fi.exists()){
            task->file = new QFile(local);

            if(task->file->open(QIODevice::ReadOnly)){
                task->filesize = fi.size();
                task->readysize = 0l;
                QDateTime dt = fi.fileTime(QFileDevice::FileModificationTime);
                //dt.setTimeZone()
                //QString url = QString("{tpl}://%1:%2%3").arg(this->host).arg(this->port).arg(remote);
                QString url = QString("{tpl}://%1%2").arg(this->host).arg(remote);
                struct curl_slist *cmdlist = NULL;
                if(this->mfmt){
                    cmdlist = curl_slist_append(cmdlist, ("MFMT " + dt.toString("yyyyMMddHHmmss") + " "+remote).toStdString().c_str());
                }else{
                    cmdlist = curl_slist_append(cmdlist, ("MDTM " + dt.toString("yyyyMMddHHmmss") + " "+remote).toStdString().c_str());
                }
                this->setOption(CURLOPT_CUSTOMREQUEST,nullptr);
                this->setOption(CURLOPT_READDATA,(void*)task);
                this->setOption(CURLOPT_READFUNCTION,network_read_callback);
                this->setOption(CURLOPT_UPLOAD,1L);
                this->setOption(CURLOPT_URL,url.toStdString().c_str());
                this->setOption(CURLOPT_INFILESIZE_LARGE,(curl_off_t)(task->filesize));
                //this->setOption(CURLOPT_{TPL}PORT,"-");
                this->setOption(CURLOPT_{TPL}_CREATE_MISSING_DIRS,CURL{TPL}_CREATE_DIR);
                //this->setOption(CURLOPT_PORT,0);
                this->setOption(CURLOPT_CONNECTTIMEOUT,COMMAND_TIMEOUT);
                this->setOption(CURLOPT_TIMEOUT,(std::max)((int)task->filesize/1024*20,3));
                //this->setOption(CURLOPT_NOSIGNAL,1);
                //this->setOption(CURLOPT_{TPL}_SKIP_PASV_IP,1);
                //qDebug()<<"timeout:"<<(std::max)((int)task->filesize/1024*20,3);
                this->setOption(CURLOPT_POSTQUOTE, cmdlist);
                response->setCommand("UPLOAD "+local);
                this->errorCode = this->access(response);
                //response->debug();
                this->header = response->header;;
                this->body = response->body;
                response->parse();
                task->file->close();

                QString host = "{tpl}://" + this->host;
                host += "/";
                this->setOption(CURLOPT_URL, host.toStdString().c_str());
                this->setOption(CURLOPT_READFUNCTION, nullptr);
                this->setOption(CURLOPT_UPLOAD, 0L);
                this->setOption(CURLOPT_READDATA, nullptr);
                this->setOption(CURLOPT_INFILESIZE_LARGE, (curl_off_t)0);
                this->setOption(CURLOPT_PORT,this->port);
                this->setOption(CURLOPT_{TPL}PORT, nullptr);
                this->setOption(CURLOPT_{TPL}_CREATE_MISSING_DIRS,CURL{TPL}_CREATE_DIR_NONE);
                this->setOption(CURLOPT_TIMEOUT, COMMAND_TIMEOUT);
                this->setOption(CURLOPT_POSTQUOTE, nullptr);
            }else{
                response->errorCode = -1;
                response->errorMsg = QObject::tr("File read failed");
            }
            delete task->file;
            task->file = nullptr;
        }else{
            response->errorCode = -2;
            response->errorMsg = QObject::tr("File is not exists");
        }
        return response;
    }

    NetworkResponse* {TPL}::download(Task* task)
    {
        QMutexLocker locker(&mutex);
        QString local = task->local;
        QString remote = task->remote;
        NetworkResponse *response =  new {TPL}Response;
        QFileInfo fi(local);
        QDir d = fi.dir();
        bool status = true;
        if(!d.exists()){
            status = d.mkpath(d.path());
        }
        if(!status){
            response->errorCode = -1;
            response->errorMsg = QObject::tr("Folder is not exists");
        }else{
            task->file = new QFile(local);
            if(task->file->open(QIODevice::WriteOnly)){
                QString url = QString("{tpl}://%1%2").arg(this->host).arg(remote);
                this->setOption(CURLOPT_WRITEDATA, (void*)task->file);
                this->setOption(CURLOPT_WRITEFUNCTION, network_write_callback);
                this->setOption(CURLOPT_XFERINFOFUNCTION, network_progress_callback);
                this->setOption(CURLOPT_XFERINFODATA, (void*)task);
                this->setOption(CURLOPT_NOPROGRESS, 0L);
                this->setOption(CURLOPT_URL, url.toStdString().c_str());
                this->setOption(CURLOPT_TIMEOUT, 0);

                response->setCommand("DOWNLOAD "+remote);

                this->errorCode = this->access(response,false);
                this->header = response->header;;
                this->body = response->body;
                response->parse();
                task->file->close();

                this->setOption(CURLOPT_NOPROGRESS, 1L);
                QString host = "{tpl}://" + this->host;
                host += "/";
                this->setOption(CURLOPT_URL,host.toStdString().c_str());
                this->setOption(CURLOPT_PORT,this->port);
                this->setOption(CURLOPT_TIMEOUT,COMMAND_TIMEOUT);
                this->setOption(CURLOPT_CONNECTTIMEOUT,COMMAND_TIMEOUT);
            }else{
                response->errorCode = -2;
                response->errorMsg = QObject::tr("File write failed");
            }
            response->debug();
            delete task->file;
            task->file = nullptr;
        }

        return response;
    }

    NetworkResponse* {TPL}::del(Task* task)
    {
        if(task->type==0){
            QMutexLocker locker(&mutex);
            QString command = QString("DELE %1").arg(task->remote);
            return this->sendCommand(command);
        }else{
            return this->rmDir(task->remote);
        }
    }

    NetworkResponse* {TPL}::chmod(Task* task)
    {
        QMutexLocker locker(&mutex);
        if(task->data.contains(Task::MODE)){
            QString command = QString("SITE CHMOD %1 %2").arg(task->data[Task::MODE].toInt()).arg(task->remote);
            return this->sendCommand(command);
        }else{
            NetworkResponse *response =  new {TPL}Response;
            response->errorCode = -1;
            response->errorMsg = QObject::tr("Invalid file mode");
            return response;
        }
    }

    NetworkResponse* {TPL}::chmod(QString dst,int mode)
    {
        QMutexLocker locker(&mutex);
        QString command = QString("SITE CHMOD %1 %2").arg(mode).arg(dst);
        return this->sendCommand(command);
    }

    NetworkResponse* {TPL}::del(QString dst)
    {
        QMutexLocker locker(&mutex);
        QString command = QString("DELE %1").arg(dst);
        return this->sendCommand(command);
    }

    NetworkResponse* {TPL}::setAscii()
    {
        QString command = "TYPE A";
        return this->sendSyncCommand(command);
    }

    NetworkResponse* {TPL}::setBinary()
    {
        QString command = "TYPE I";
        return this->sendSyncCommand(command);
    }

    NetworkResponse* {TPL}::setPassive(bool pasv)
    {
        if (pasv) {
            QString command = "PASV";
            return this->sendSyncCommand(command);
        }else{
            QString command = "PORT";
            return this->sendSyncCommand(command);
            //return nullptr;
        }
    }

    NetworkResponse* {TPL}::chDir(const QString &dir)
    {
        QString command = "CWD "+dir;
        return this->sendSyncCommand(command);
    }

    NetworkResponse* {TPL}::mkDir(const QString &dir)
    {
        QString command = "MKD "+dir;
        return this->sendSyncCommand(command);
    }

    NetworkResponse* {TPL}::rmDir(const QString &dir)
    {
        QString command = "RMD "+dir;
        return this->sendSyncCommand(command);
    }

    NetworkResponse* {TPL}::pwd()
    {
        QMutexLocker locker(&mutex);
        return nullptr;
    }

    NetworkResponse* {TPL}::rename(QString src,QString dst)
    {
        QMutexLocker locker(&mutex);
        QString command = QString("RNFR %1").arg(src);
        NetworkResponse *response = this->sendCommand(command);
        if(response->status()){
            delete response;
            command = QString("RNTO %1").arg(dst);
            response = this->sendCommand(command);
        }
        return response;
    }

    NetworkResponse* {TPL}::customeAccess(QString name,QMap<QString,QVariant> data)
    {
        //return this->errorCode;
        return nullptr;
    }
}

