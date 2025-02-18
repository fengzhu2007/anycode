#include "sftp.h"
#include "sftp_response.h"
#include "transfer/task.h"
#include "sftp_setting.h"
#include <common/utils.h>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QDebug>
namespace ady {
    SFTP::SFTP(CURL* curl,long long id)
        :NetworkRequest(curl,id)
    {
        this->m_setting = nullptr;
        this->connected = false;
        this->setOption(CURLOPT_TIMEOUT,COMMAND_TIMEOUT);
    }


    SFTP::~SFTP(){
        if(m_setting!=nullptr){
            delete m_setting;
            m_setting = nullptr;
        }
    }

    int SFTP::access(NetworkResponse* response,bool body)
    {
        this->setHeaderHolder(&response->header);
        if(body){
            this->setBodyHolder(&response->body);
        }
        CURLcode res = curl_easy_perform(this->curl);
        if(res!=CURLE_OK){
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





    void SFTP::init(const SiteRecord& info){
        this->setHost(info.host);
        this->setPort(info.port);
        this->setUsername(info.username);
        this->setPassword(info.password);
        m_rootPath = info.path;
        if(m_setting!=nullptr){
            delete m_setting;
        }
        m_setting = new SFTPSetting(info.data);
        m_dirMapping = m_setting->dirMapping();
        m_uploadCommands = m_setting->uploadCommands();
    }

    NetworkResponse* SFTP::link()
    {
        QMutexLocker locker(&mutex);
        QString host = "sftp://" + this->host;
        host += "/";
        this->setOption(CURLOPT_URL,host.toStdString().c_str());
        this->setOption(CURLOPT_PORT,this->port);
        this->setOption(CURLOPT_TIMEOUT,COMMAND_TIMEOUT);
        this->setOption(CURLOPT_CONNECTTIMEOUT,COMMAND_TIMEOUT);
        this->setOption(CURLOPT_USERNAME,this->username.toStdString().c_str());
        this->setOption(CURLOPT_PASSWORD,this->password.toStdString().c_str());
        NetworkResponse* response = new SFTPResponse(this->id);
        int ret = this->access(response);
        if(ret==0){
            this->connected = true;
            this->initEnv();
        }
        //delete response;
        return response;
    }

    NetworkResponse* SFTP::unlink()
    {
        /*QMutexLocker locker(&mutex);
        this->setOption(CURLOPT_TIMEOUT,UNLINK_TIMEOUT);
        QString command = "Bye";
        NetworkResponse* response = this->sendCommand(command);
        this->setOption(CURLOPT_TIMEOUT,COMMAND_TIMEOUT);
        response->debug();
        return response;*/
        NetworkResponse* response = new SFTPResponse(this->id);
        response->errorCode = 0;
        return response;
    }


    NetworkResponse* SFTP::sendSyncCommand(const QString& command)
    {
        QMutexLocker locker(&mutex);
        return this->sendCommand(command);
    }

    void SFTP::initEnv()
    {
        return ;
    }


    NetworkResponse* SFTP::sendCommand(const QString& command)
    {
        struct curl_slist *headerlist = NULL;
        if(!command.isEmpty()){
            headerlist = curl_slist_append(headerlist, command.toStdString().c_str());
        }
        //qDebug()<<"exec command:"<<command;
        this->setOption(CURLOPT_USERNAME,this->username.toStdString().c_str());
        this->setOption(CURLOPT_PASSWORD,this->password.toStdString().c_str());
        this->setOption(CURLOPT_POSTQUOTE,headerlist);
        SFTPResponse* response = new SFTPResponse(this->id);
        response->setCommand(command);
        this->errorCode = this->access(response);
        this->header = response->header;;
        this->body = response->body;
        response->parse();

        if(headerlist!=NULL){
            curl_slist_free_all(headerlist);
        }
        this->setOption(CURLOPT_POSTQUOTE,NULL);
        response->setCommand(command);
        return response;
    }

    NetworkResponse* SFTP::listDir(const QString& dir,int page,int pageSize)
    {
        Q_UNUSED(page);
        Q_UNUSED(pageSize);
        QMutexLocker locker(&mutex);

        QString url = "sftp://"+this->host;
        if(!dir.startsWith("/")){
            url += "/";
        }
        url += this->escape(dir);
        this->setOption(CURLOPT_URL,url.toStdString().c_str());
        this->setOption(CURLOPT_NOBODY,0L);
        QString command ;
        NetworkResponse *response = this->sendCommand(command);
        if(response->errorCode==0){
            response->params["dir"] = dir;
        }
        response->setCommand(QLatin1String("LIST %1").arg(dir));
        return response;

    }

    NetworkResponse* SFTP::tinyListDir(const QString& dir)
    {
        QMutexLocker locker(&mutex);
        QString url = "sftp://"+this->host;
        if(!dir.startsWith("/")){
            url += "/";
        }
        url += this->escape(dir);
        this->setOption(CURLOPT_URL,url.toStdString().c_str());
        this->setOption(CURLOPT_NOBODY,0L);
        QString command ;
        NetworkResponse* response =  this->sendCommand(command);
        if(response->errorCode==0){
            response->params["dir"] = dir;
        }
        response->setCommand(QLatin1String("LIST %1").arg(dir));
        return response;
    }

    NetworkResponse* SFTP::upload(Task* task)
    {
        QMutexLocker locker(&mutex);
        QString local = task->local;
        QString remote = task->remote;
        NetworkResponse *response =  new SFTPResponse(this->id);
        QFileInfo fi(local);
        if(fi.exists()){
            task->file = new QFile(local);

            if(task->file->open(QIODevice::ReadOnly)){
                task->filesize = fi.size();
                task->readysize = 0l;
                //qDebug()<<"size:"<<task->filesize;
                QDateTime dt = fi.fileTime(QFileDevice::FileModificationTime);
                //QString url = QString("sftp://%1:%2%3").arg(this->host).arg(this->port).arg(remote);
                //QString url = QString("sftp://%1%2").arg(this->host).arg(this->escape(remote));
                QString url = QLatin1String("sftp://%1").arg(this->host);
                if(!remote.startsWith("/")){
                    url += "/";
                }
                url += this->escape(remote);
                struct curl_slist *cmdlist = NULL;
                QString cmd = QString("mtime \"%1\" \"%2\"").arg(dt.toString(Qt::RFC2822Date)).arg(remote);
                cmdlist = curl_slist_append(cmdlist,cmd.toStdString().c_str());//mtime date file
                Q_FOREACH(QString cmd,m_uploadCommands){
                    cmdlist = curl_slist_append(cmdlist, fixedCommandPath(cmd,remote).toStdString().c_str());
                }
                this->setOption(CURLOPT_CUSTOMREQUEST,nullptr);
                this->setOption(CURLOPT_READDATA,(void*)task);
                this->setOption(CURLOPT_READFUNCTION,network_read_callback);
                this->setOption(CURLOPT_UPLOAD,1L);
                //this->setOption(CURLOPT_NOBODY,1L);
                this->setOption(CURLOPT_URL,url.toStdString().c_str());
                this->setOption(CURLOPT_INFILESIZE_LARGE,(curl_off_t)(task->filesize));
                //this->setOption(CURLOPT_SFTPPORT,"-");
                this->setOption(CURLOPT_FTP_CREATE_MISSING_DIRS,CURLFTP_CREATE_DIR);
                //this->setOption(CURLOPT_PORT,0);
                this->setOption(CURLOPT_CONNECTTIMEOUT,COMMAND_TIMEOUT);
                this->setOption(CURLOPT_TIMEOUT,(std::max)((int)task->filesize/1024*20,3));
                //this->setOption(CURLOPT_NOSIGNAL,1);
                //this->setOption(CURLOPT_SFTP_SKIP_PASV_IP,1);
                //qDebug()<<"timeout:"<<(std::max)((int)task->filesize/1024*20,3);
                this->setOption(CURLOPT_POSTQUOTE, cmdlist);
                response->setCommand("UPLOAD "+local);
                this->errorCode = this->access(response);
                //response->debug();
                this->header = response->header;;
                this->body = response->body;
                response->parse();
                task->file->close();

                QString host = "sftp://" + this->host;
                host += "/";
                this->setOption(CURLOPT_URL, host.toStdString().c_str());
                this->setOption(CURLOPT_READFUNCTION, nullptr);
                this->setOption(CURLOPT_UPLOAD, 0L);
                this->setOption(CURLOPT_READDATA, nullptr);
                this->setOption(CURLOPT_INFILESIZE_LARGE, (curl_off_t)0);
                this->setOption(CURLOPT_PORT,this->port);
                //this->setOption(CURLOPT_SFTPPORT, nullptr);
                //this->setOption(CURLOPT_SFTP_CREATE_MISSING_DIRS,CURLSFTP_CREATE_DIR_NONE);
                this->setOption(CURLOPT_TIMEOUT, COMMAND_TIMEOUT);
                this->setOption(CURLOPT_POSTQUOTE, nullptr);

                if(cmdlist!=NULL){
                    curl_slist_free_all(cmdlist);
                }

                if(task->abort==true){
                    response->errorCode = -3;
                    response->errorMsg = QObject::tr("User aborted");
                    task->abort = false;
                }

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

    NetworkResponse* SFTP::download(Task* task)
    {
        QMutexLocker locker(&mutex);
        QString local = task->local;
        QString remote = task->remote;
        NetworkResponse *response =  new SFTPResponse(this->id);
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
                //QString url = QString("sftp://%1%2").arg(this->host).arg(this->escape(remote));
                QString url = QLatin1String("sftp://%1").arg(this->host);
                if(!remote.startsWith("/")){
                    url += "/";
                }
                url += this->escape(remote);
                this->setOption(CURLOPT_WRITEDATA, (void*)task->file);
                this->setOption(CURLOPT_WRITEFUNCTION, network_write_callback);
                this->setOption(CURLOPT_XFERINFOFUNCTION, network_progress_callback);
                this->setOption(CURLOPT_XFERINFODATA, (void*)task);
                this->setOption(CURLOPT_NOPROGRESS, 0L);
                //this->setOption(CURLOPT_NOBODY,1L);
                this->setOption(CURLOPT_URL, url.toStdString().c_str());
                this->setOption(CURLOPT_TIMEOUT, 0);

                response->setCommand("DOWNLOAD "+remote);

                this->errorCode = this->access(response,false);
                this->header = response->header;;
                this->body = response->body;
                response->parse();
                task->file->close();

                this->setOption(CURLOPT_NOPROGRESS, 1L);
                QString host = "sftp://" + this->host;
                host += "/";
                this->setOption(CURLOPT_URL,host.toStdString().c_str());
                //this->setOption(CURLOPT_PORT,this->port);
                this->setOption(CURLOPT_TIMEOUT,COMMAND_TIMEOUT);
                this->setOption(CURLOPT_CONNECTTIMEOUT,COMMAND_TIMEOUT);


                if(task->abort==true){
                    response->errorCode = -3;
                    response->errorMsg = QObject::tr("User aborted");
                    task->abort = false;
                }

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

    NetworkResponse* SFTP::del(Task* task)
    {
        if(task->type==0){
            QMutexLocker locker(&mutex);
            QString command = QString("rm \"%1\"").arg(task->remote);
            //qDebug()<<"del command:"<<command;
            return this->sendCommand(command);
        }else{
            return this->rmDir(task->remote);
        }
    }

    NetworkResponse* SFTP::chmod(Task* task)
    {
        QMutexLocker locker(&mutex);
        if(task->data.contains(Task::MODE)){
            QString command = QString("chmod  %1 \"%2\"").arg(task->data[Task::MODE].toInt()).arg(task->remote);
            return this->sendCommand(command);
        }else{
            NetworkResponse *response =  new SFTPResponse(this->id);
            response->errorCode = -1;
            response->errorMsg = QObject::tr("Invalid file mode");
            return response;
        }
    }

    NetworkResponse* SFTP::chmod(const QString& dst,int mode)
    {
        QMutexLocker locker(&mutex);
        QString command = QString("chmod %1 \"%2\"").arg(mode).arg(dst);
        return this->sendCommand(command);
    }

    NetworkResponse* SFTP::del(const QString& dst)
    {
        QMutexLocker locker(&mutex);
        QString command = QString("rm \"%1\"").arg(dst);
        //qDebug()<<"del command2:"<<command;
        return this->sendCommand(command);
    }

    /*NetworkResponse* SFTP::setAscii()
    {
        QString command = "TYPE A";
        return this->sendSyncCommand(command);
    }

    NetworkResponse* SFTP::setBinary()
    {
        QString command = "TYPE I";
        return this->sendSyncCommand(command);
    }

    NetworkResponse* SFTP::setPassive(bool pasv)
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

    NetworkResponse* SFTP::chDir(const QString &dir)
    {
        QString command = "CWD "+dir;
        return this->sendSyncCommand(command);
    }*/

    NetworkResponse* SFTP::mkDir(const QString &dir)
    {
        QMutexLocker locker(&mutex);
        QString command = QString("mkdir \"%1\"").arg(dir);
        struct curl_slist *headerlist = NULL;
        headerlist = curl_slist_append(headerlist, command.toStdString().c_str());
        Q_FOREACH(QString cmd,m_uploadCommands){
            QString cc = fixedCommandPath(cmd,dir);
            headerlist = curl_slist_append(headerlist, cc.toStdString().c_str());
        }
        this->setOption(CURLOPT_FTP_USE_EPSV,1);
        this->setOption(CURLOPT_USERNAME,this->username.toStdString().c_str());
        this->setOption(CURLOPT_PASSWORD,this->password.toStdString().c_str());
        this->setOption(CURLOPT_NOBODY,1L);
        this->setOption(CURLOPT_HEADER,1L);
        this->setOption(CURLOPT_POSTQUOTE,headerlist);
        SFTPResponse* response = new SFTPResponse(this->id);
        response->setCommand(command);
        this->errorCode = this->access(response);
        this->header = response->header;;
        this->body = response->body;
        response->parse();

        if(headerlist!=NULL){
            curl_slist_free_all(headerlist);
        }
        this->setOption(CURLOPT_POSTQUOTE,nullptr);
        //response->debug();
        return response;
    }

    NetworkResponse* SFTP::rmDir(const QString &dir)
    {
        QString command = QString("rmdir \"%1\"").arg(dir);
        this->setOption(CURLOPT_NOBODY,1L);
        return this->sendSyncCommand(command);
    }

    NetworkResponse* SFTP::pwd()
    {
        QMutexLocker locker(&mutex);
        return nullptr;
    }

    NetworkResponse* SFTP::rename(const QString& src,const QString& dst)
    {
        QMutexLocker locker(&mutex);
        this->setOption(CURLOPT_NOBODY,1L);
        QString command = QString("rename \"%1\" \"%2\"").arg(this->escape(src)).arg(this->escape(dst));
        //QString command = QString("RNFR %1").arg(src);
        NetworkResponse *response = this->sendCommand(command);
        /*if(response->status()){
            delete response;
            command = QString("RNTO %1").arg(dst);
            response = this->sendCommand(command);
        }*/
        response->debug();
        return response;
    }

    void SFTP::setUploadCommands(QStringList commands)
    {
        m_uploadCommands = commands;
    }

    QString SFTP::matchToPath(const QString& from,bool is_file,bool local){
        Q_UNUSED(is_file);
        if(m_dirMapping.size()>0){
            QString ret;
            if(local){
                //local to remote
                for(auto one:m_dirMapping){
                    const QString localPath = one.first;//like  path1/path2/
                    const QString remotePath = one.second;//like path3/path4/
                    if(from.startsWith(localPath)){
                        ret = remotePath + from.mid(localPath.length());
                        break;
                    }
                }
                if(ret.isEmpty()){
                    ret = from;
                }
            }else{
                //remote to local
                for(auto one:m_dirMapping){
                    const QString localPath = one.first;//like  path1/path2/
                    const QString remotePath = one.second;//like path3/path4/
                    if(from.startsWith(remotePath)){
                        ret = localPath + from.mid(localPath.length());
                        break;
                    }
                }
                if(ret.isEmpty()){
                    ret = from;
                }
            }
            return ret;//new relative path
        }else{
            return from;
        }
    }

    NetworkResponse* SFTP::customeAccess(const QString& name,QMap<QString,QVariant> data)
    {
        //return this->errorCode;
        return nullptr;
    }

    QString SFTP::fixedCommandPath(QString &command,const QString &path)
    {
        return command.replace("${path}",QString("\"%1\"").arg(path));
    }
}

