#include "ftp.h"
#include "ftp_response.h"
#include "transfer/task.h"
#include "common/utils.h"
#include "ftp_setting.h"
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QTimeZone>
#include <QDebug>
namespace ady {
    FTP::FTP(CURL* curl,long long id)
        :NetworkRequest(curl,id)
    {
        this->connected = false;
        this->mlsd = false;
        this->mfmt = false;
        this->utf8 = false;
        this->window_nt = false;
        this->setOption(CURLOPT_TIMEOUT,COMMAND_TIMEOUT);
        this->m_setting = nullptr;
    }

    FTP::~FTP(){
        if(m_setting!=nullptr){
            delete m_setting;
            m_setting = nullptr;
        }
    }

    void FTP::init(const SiteRecord& info){
        this->setHost(info.host);
        this->setPort(info.port);
        this->setUsername(info.username);
        this->setPassword(info.password);
        m_rootPath = info.path;
        if(m_setting!=nullptr){
            delete m_setting;
        }
        m_setting = new FTPSetting(info.data);
        m_dirMapping = m_setting->dirMapping();
    }

    int FTP::access(NetworkResponse* response,bool body)
    {
        this->setHeaderHolder(&response->header);
        if(body){
            this->setBodyHolder(&response->body);
        }
        CURLcode res = curl_easy_perform(this->curl);
        if(res!=CURLE_OK && res != CURLE_FTP_COULDNT_RETR_FILE ){
            this->errorMsg = curl_easy_strerror(res);
            this->errorCode = (int)res;
        }else{
            this->errorMsg = "";
            this->errorCode = 0;
        }
        m_last_request_time = QDateTime::currentMSecsSinceEpoch();
        response->errorCode = this->errorCode;
        response->errorMsg = this->errorMsg;
        response->parse();
        return (int)res;
    }

    NetworkResponse* FTP::link()
    {
        QMutexLocker locker(&mutex);
        QString host = "ftp://" + this->host;
        //if(!m_rootPath.startsWith("/")){
        //    host += "/";
        //}
        //host += m_rootPath;
        host += "/";
        this->setOption(CURLOPT_URL,host.toStdString().c_str());
        this->setOption(CURLOPT_PORT,this->port);
        this->setOption(CURLOPT_TIMEOUT,COMMAND_TIMEOUT);
        this->setOption(CURLOPT_CONNECTTIMEOUT,COMMAND_TIMEOUT);
        this->setOption(CURLOPT_USERNAME,this->username.toStdString().c_str());
        this->setOption(CURLOPT_PASSWORD,this->password.toStdString().c_str());
        NetworkResponse* response = new FTPResponse(this->id);
        int ret = this->access(response);
        if(ret==0){
            this->connected = true;
            this->initEnv();
        }
        //delete response;
        return response;
    }

    NetworkResponse* FTP::unlink()
    {
        QMutexLocker locker(&mutex);
        this->setOption(CURLOPT_TIMEOUT,UNLINK_TIMEOUT);
        QString command = "QUIT";
        NetworkResponse* response = this->sendCommand(command,false);
        this->setOption(CURLOPT_TIMEOUT,COMMAND_TIMEOUT);
        return response;
    }


    NetworkResponse* FTP::sendSyncCommand(const QString& command,bool pre_utf8)
    {
        QMutexLocker locker(&mutex);
        return this->sendCommand(command,pre_utf8);
    }

    void FTP::initEnv()
    {
        {
            NetworkResponse* response = this->sendCommand("SYST",false);
            if(response->status()){
                //qDebug()<<"system os:"<<response->networkErrorMsg;
                if(response->networkErrorMsg.toUpper()=="WINDOWS_NT"){
                    this->window_nt = true;
                }
            }
        }

        {

            NetworkResponse* response = this->sendCommand("FEAT",false);
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

                if(this->utf8 && this->window_nt){
                    NetworkResponse* response = this->sendCommand("OPTS UTF8 ON",false);
                    delete response;
                }
            }

        }


    }


    NetworkResponse* FTP::sendCommand(const QString& command,bool pre_utf8)
    {
        //qDebug()<<"exec command:"<<command;
        struct curl_slist *precmdlist = NULL;
        if(pre_utf8){
            if(this->utf8 && this->window_nt){
                precmdlist = curl_slist_append(precmdlist, "OPTS UTF8 ON");
                this->setOption(CURLOPT_QUOTE, precmdlist);
            }
        }
        this->setOption(CURLOPT_USERNAME,this->username.toStdString().c_str());
        this->setOption(CURLOPT_PASSWORD,this->password.toStdString().c_str());
        this->setOption(CURLOPT_CUSTOMREQUEST,command.toStdString().c_str());
        FTPResponse* response = new FTPResponse(this->id);
        response->setCommand(command);
        this->errorCode = this->access(response);
        this->header = response->header;;
        this->body = response->body;
        response->parse();


        this->setOption(CURLOPT_QUOTE,nullptr);
        if(precmdlist!=NULL){
            curl_slist_free_all(precmdlist);
        }
        return response;
    }

    NetworkResponse* FTP::listDir(const QString& dir,int page,int pageSize)
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

    NetworkResponse* FTP::tinyListDir(const QString& dir)
    {
        QMutexLocker locker(&mutex);
        QString command = "LIST "+dir/*+" -al"*/;//windows error
        NetworkResponse* response =  this->sendCommand(command);
        if(response->errorCode==0){
            response->params["dir"] = dir;
        }
        return response;
    }

    NetworkResponse* FTP::upload(Task* task)
    {
        QMutexLocker locker(&mutex);
        QString local = task->local;
        QString remote = task->remote;
        NetworkResponse *response =  new FTPResponse(this->id);
        QFileInfo fi(local);
        if(fi.exists()){
            task->file = new QFile(local);

            if(task->file->open(QIODevice::ReadOnly)){
                task->filesize = fi.size();
                task->readysize = 0l;
                QDateTime dt = fi.fileTime(QFileDevice::FileModificationTime);
                //dt.setTimeZone(QTimeZone::utc());
                //dt.setTimeZone()
                //QString url = QString("ftp://%1:%2%3").arg(this->host).arg(this->port).arg(remote);
                QString url = QString("ftp://%1%2").arg(this->host).arg(this->escape(remote));
                //qDebug()<<"url:"<<dt.toString("yyyyMMddHHmmss");
                struct curl_slist *precmdlist = NULL;
                if(this->utf8 && this->window_nt){
                    precmdlist = curl_slist_append(precmdlist, "OPTS UTF8 ON");
                    this->setOption(CURLOPT_QUOTE, precmdlist);
                }

                struct curl_slist *cmdlist = NULL;
                if(this->mfmt){
                    cmdlist = curl_slist_append(cmdlist, ("MFMT " + dt.toString("yyyyMMddHHmmss") + " "+(remote)+"").toStdString().c_str());
                }else{
                    cmdlist = curl_slist_append(cmdlist, ("MDTM " + dt.toString("yyyyMMddHHmmss") + " "+(remote)+"").toStdString().c_str());
                }
                this->setOption(CURLOPT_CUSTOMREQUEST,nullptr);
                this->setOption(CURLOPT_READDATA,(void*)task);
                this->setOption(CURLOPT_READFUNCTION,network_read_callback);
                this->setOption(CURLOPT_UPLOAD,1L);
                this->setOption(CURLOPT_URL,url.toStdString().c_str());
                this->setOption(CURLOPT_INFILESIZE_LARGE,(curl_off_t)(task->filesize));
                //this->setOption(CURLOPT_FTPPORT,"-");
                this->setOption(CURLOPT_FTP_CREATE_MISSING_DIRS,CURLFTP_CREATE_DIR);
                //this->setOption(CURLOPT_PORT,0);
                this->setOption(CURLOPT_CONNECTTIMEOUT,COMMAND_TIMEOUT);
                this->setOption(CURLOPT_TIMEOUT,(std::max)((int)task->filesize/1024*20,3));
                //this->setOption(CURLOPT_NOSIGNAL,1);
                //this->setOption(CURLOPT_FTP_SKIP_PASV_IP,1);
                //qDebug()<<"timeout:"<<(std::max)((int)task->filesize/1024*20,3);
                this->setOption(CURLOPT_POSTQUOTE, cmdlist);
                response->setCommand("UPLOAD "+local);
                this->errorCode = this->access(response);
                //response->debug();
                if(response->errorCode==21 && response->networkErrorCode==550){
                    QString head = response->headers[response->headers.length() - 3];
                    QStringList arr = head.split(" ");
                    if(arr.size()>0){
                        response->networkErrorCode = arr[0].toInt();
                        response->errorCode = 0;
                    }
                    //qDebug()<<"head:"<<head<<";"<<response->networkErrorCode;
                }
                //response->status();

                this->header = response->header;;
                this->body = response->body;
                //response->parse();
                task->file->close();

                QString host = "ftp://" + this->host;
                host += "/";
                this->setOption(CURLOPT_URL, host.toStdString().c_str());
                this->setOption(CURLOPT_READFUNCTION, nullptr);
                this->setOption(CURLOPT_UPLOAD, 0L);
                this->setOption(CURLOPT_READDATA, nullptr);
                this->setOption(CURLOPT_INFILESIZE_LARGE, (curl_off_t)0);
                this->setOption(CURLOPT_PORT,this->port);
                this->setOption(CURLOPT_FTPPORT, nullptr);
                this->setOption(CURLOPT_FTP_CREATE_MISSING_DIRS,CURLFTP_CREATE_DIR_NONE);
                this->setOption(CURLOPT_TIMEOUT, COMMAND_TIMEOUT);
                this->setOption(CURLOPT_POSTQUOTE, nullptr);
                this->setOption(CURLOPT_QUOTE,nullptr);
                if(precmdlist!=NULL){
                    curl_slist_free_all(precmdlist);
                }
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

    NetworkResponse* FTP::download(Task* task)
    {
        QMutexLocker locker(&mutex);
        QString local = task->local;
        QString remote = task->remote;
        NetworkResponse *response =  new FTPResponse(this->id);
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
                QString url = QString("ftp://%1%2").arg(this->host).arg(this->escape(remote));

                struct curl_slist *precmdlist = NULL;
                if(this->utf8 && this->window_nt){
                    precmdlist = curl_slist_append(precmdlist, "OPTS UTF8 ON");
                    this->setOption(CURLOPT_QUOTE, precmdlist);
                }

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
                QString host = "ftp://" + this->host;
                host += "/";
                this->setOption(CURLOPT_URL,host.toStdString().c_str());
                this->setOption(CURLOPT_PORT,this->port);
                this->setOption(CURLOPT_TIMEOUT,COMMAND_TIMEOUT);
                this->setOption(CURLOPT_CONNECTTIMEOUT,COMMAND_TIMEOUT);
                this->setOption(CURLOPT_QUOTE,nullptr);
                if(precmdlist!=NULL){
                    curl_slist_free_all(precmdlist);
                }
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

    NetworkResponse* FTP::del(Task* task)
    {
        if(task->type==0){
            QMutexLocker locker(&mutex);
            QString command = QString("DELE %1").arg(task->remote);
            return this->sendCommand(command);
        }else{
            return this->rmDir(task->remote);
        }
    }

    NetworkResponse* FTP::chmod(Task* task)
    {
        QMutexLocker locker(&mutex);
        if(task->data.contains(Task::MODE)){
            QString command = QString("SITE CHMOD %1 %2").arg(task->data[Task::MODE].toInt()).arg(task->remote);
            return this->sendCommand(command);
        }else{
            NetworkResponse *response =  new FTPResponse(this->id);
            response->errorCode = -1;
            response->errorMsg = QObject::tr("Invalid file mode");
            return response;
        }
    }

    NetworkResponse* FTP::chmod(const QString& dst,int mode)
    {
        QMutexLocker locker(&mutex);
        QString command = QString("SITE CHMOD %1 %2").arg(mode).arg(dst);
        return this->sendCommand(command);
    }

    QString FTP::matchToPath(const QString& from,bool is_file,bool local){
        //from is relative path like: path1/path2/file.html
        Q_UNUSED(is_file);
        if(m_dirMapping.size()>0){
            QString ret;
            if(local){
                //local to remote
                for(auto one:m_dirMapping){
                    const QString localPath =  one.first.startsWith("/")?one.first.mid(1):one.first;//like  path1/path2/
                    const QString remotePath = one.second.startsWith("/")?one.second.mid(1):one.second;//like path3/path4/
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
                    const QString localPath =  one.first.startsWith("/")?one.first.mid(1):one.first;//like  path1/path2/
                    const QString remotePath = one.second.startsWith("/")?one.second.mid(1):one.second;//like path3/path4/
                    if(from.startsWith(remotePath)){
                        ret = localPath + from.mid(remotePath.length());
                        break;
                    }
                }
                if(ret.isEmpty()){
                    ret = from;
                }
            }
            if(ret.startsWith("/")){
                ret = ret.mid(1);
            }
            return ret;//new relative path
        }else{
            return from;
        }
    }

    void FTP::autoClose(long long current){
        if(this->connected && current > m_setting->interval() + m_last_request_time){
            qDebug()<<"auto close";
            this->unlink();
        }
    }


    NetworkResponse* FTP::setAscii()
    {
        QString command = "TYPE A";
        return this->sendSyncCommand(command,false);
    }

    NetworkResponse* FTP::setBinary()
    {
        QString command = "TYPE I";
        return this->sendSyncCommand(command,false);
    }

    NetworkResponse* FTP::setPassive(bool pasv)
    {
        if (pasv) {
            QString command = "PASV";
            return this->sendSyncCommand(command,false);
        }else{
            QString command = "PORT";
            return this->sendSyncCommand(command,false);
            //return nullptr;
        }
    }

    NetworkResponse* FTP::chDir(const QString &dir)
    {
        QString command = "CWD "+dir;
        return this->sendSyncCommand(command);
    }

    NetworkResponse* FTP::del(const QString& path){
        QMutexLocker locker(&mutex);
        QString command = QString("DELE %1").arg(path);
        return this->sendCommand(command);
    }



    NetworkResponse* FTP::mkDir(const QString &dir)
    {
        QString command = "MKD "+dir;
        return this->sendSyncCommand(command);
    }

    NetworkResponse* FTP::rmDir(const QString &dir)
    {
        QString command = "RMD "+dir;
        return this->sendSyncCommand(command);
    }

    NetworkResponse* FTP::pwd()
    {
        QMutexLocker locker(&mutex);
        return nullptr;
    }

    NetworkResponse* FTP::rename(const QString& src,const QString& dst)
    {
        QMutexLocker locker(&mutex);
        QString command = QString("RNFR %1").arg(src);
        NetworkResponse *response = this->sendCommand(command);
        if(response->errorCode==0 && response->networkErrorCode==350){
            delete response;
            command = QString("RNTO %1").arg(dst);
            response = this->sendCommand(command);
        }
        return response;
    }

    NetworkResponse* FTP::customeAccess(const QString& name,QMap<QString,QVariant> data)
    {
        //return this->errorCode;
        return nullptr;
    }
}

