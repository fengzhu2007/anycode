#include "oss.h"
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QMimeDatabase>
#include <common/utils.h>
#include "oss_response.h"
#include "transfer/Task.h"
#include "oss_setting.h"
#include <openssl/hmac.h>
#include <openssl/opensslconf.h>
#include <locale.h>
#ifdef Q_OS_WIN
#include <hmac/hmac_local.h>
#endif

#include <QDebug>
namespace ady {
constexpr const  char OSS::METHOD[] ;
constexpr const  char OSS::OBJECT[] ;
constexpr const  char OSS::BUCKET[] ;
constexpr const  char OSS::CONTENT_TYPE[];
constexpr const  char OSS::CONTENT_MD5[] ;
constexpr const  char OSS::DATE[] ;
constexpr const  char OSS::HOST[] ;
constexpr const  char OSS::CONTENT_LENGTH[] ;
constexpr const  char OSS::AUTHORIZATION[] ;
constexpr const  char OSS::DELIMITER[] ;
constexpr const  char OSS::MARKER[] ;
constexpr const  char OSS::MAXKEYS[] ;
constexpr const  char OSS::PREFIX[] ;
constexpr const  char OSS::ACCEPT_ENCODING[] ;
OSS::OSS(CURL* curl,long long id)
    :HttpClient(curl,id)
{
    this->connected = false;
    this->m_setting = nullptr;
    this->setOption(CURLOPT_TIMEOUT,COMMAND_TIMEOUT);
}

OSS::~OSS(){
    if(this->m_setting!=nullptr){
        delete m_setting;
    }
}



void OSS::init(const SiteRecord& info){
    this->setHost(info.host);
    this->setPort(info.port);
    this->setUsername(info.username);
    this->setPassword(info.password);
    m_rootPath = info.path;
    if(m_setting!=nullptr){
        delete m_setting;
    }

    m_setting = new OSSSetting(info.data);
    m_dirMapping = m_setting->dirMapping();

    m_filters.clear();
    for(auto filter:m_setting->filterExtensions()){
        m_filters << QRegularExpression(Utils::stringToExpression(filter));
    }
}

NetworkResponse* OSS::link()
{
    QMutexLocker locker(&mutex);
    m_bucket = this->host.left(this->host.indexOf("."));
    QString dst = this->m_rootPath;
    HttpParams options;
    options[BUCKET] = m_bucket;
    options[METHOD] = "GET";
    options[OBJECT] = dst;
    QStringList arr = this->host.split(".");
    QString host = this->host;
    options[HOST] = host;
    HttpParams headers = this->signHeaders(options);
    QString prefix = this->m_rootPath;
    if(prefix.startsWith("/")){
        prefix = prefix.mid(1);
    }
    headers[PREFIX] = prefix;
    headers[MARKER] = "";
    headers[DELIMITER] = "/";
    headers[MAXKEYS] = "1";

    QStringList lists;
    HttpParamIter iter = headers.begin();
    while(iter!=headers.end()){
        lists.push_back((iter.key()+": "+iter.value()).toStdString().c_str());
        iter++;
    }
    this->addHeader(lists);
    this->setOption(CURLOPT_NOBODY,1);
    QString url = "http://"+options[HOST];
    if(!options[OBJECT].startsWith("/")){
        url += QLatin1String("/");
    }
    url += this->escape(options[OBJECT]);
    OSSResponse* response = new  OSSResponse(this->id);
    this->get(url,response);
    this->setOption(CURLOPT_NOBODY,0);
    //qDebug()<<"OBJECT"<<options[OBJECT];
    response->setCommand(QLatin1String("GET /%1").arg(prefix));

    return response;

}

NetworkResponse* OSS::unlink()
{
    QMutexLocker locker(&mutex);
    return nullptr;
}





NetworkResponse* OSS::listDir(const QString& dir,int page,int pageSize)
{
    Q_UNUSED(page);
    //Q_UNUSED(pageSize);
    QMutexLocker locker(&mutex);
    m_bucket = this->host.left(this->host.indexOf("."));
    HttpParams options;
    options[BUCKET] = m_bucket;
    options[METHOD] = "GET";
    options[OBJECT] = "/";
    QStringList arr = this->host.split(".");
    QString host = this->host;
    options[HOST] = host;
    HttpParams headers = this->signHeaders(options);
    QString prefix = this->m_rootPath;
    if(dir.isEmpty()==false){
        prefix = dir;
    }
    if(prefix.startsWith("/")){
        prefix = prefix.mid(1);
    }
    if(!prefix.endsWith("/")){
        prefix += "/";
    }
    headers[PREFIX] = prefix;
    headers[MARKER] = "";
    headers[DELIMITER] = "/";
    headers[MAXKEYS] = QString("%1").arg(pageSize);

    QStringList lists;
    HttpParamIter iter = headers.begin();
    while(iter!=headers.end()){
        lists.push_back((iter.key()+": "+iter.value()).toStdString().c_str());
        iter++;
    }
    this->addHeader(lists);
    QString url = "http://"+options[HOST];
    if(!options[OBJECT].startsWith("/")){
        url += QLatin1String("/");
    }
    url += this->escape(options[OBJECT]);
    OSSResponse* response = new  OSSResponse(this->id);
    this->get(url,response);
    response->setCommand(QLatin1String("GET /%1").arg(prefix));
     if(response->errorCode==0){
        response->params["dir"] = dir;
    }
    return response;

}

NetworkResponse* OSS::tinyListDir(const QString& dir)
{
    QMutexLocker locker(&mutex);
    m_bucket = this->host.left(this->host.indexOf("."));
    HttpParams options;
    options[BUCKET] = m_bucket;
    options[METHOD] = "GET";
    options[OBJECT] = "/";
    QStringList arr = this->host.split(".");
    QString host = this->host;
    options[HOST] = host;
    HttpParams headers = this->signHeaders(options);
    QString prefix = this->m_rootPath;
    if(dir.isEmpty()==false){
        prefix = dir;
    }
    if(prefix.startsWith("/")){
        prefix = prefix.mid(1);
    }
    if(!prefix.endsWith("/")){
        prefix += "/";
    }
    headers[PREFIX] = prefix;
    headers[MARKER] = "";
    headers[DELIMITER] = "/";
    headers[MAXKEYS] = "1000";

    QStringList lists;
    HttpParamIter iter = headers.begin();
    while(iter!=headers.end()){
        lists.push_back((iter.key()+": "+iter.value()).toStdString().c_str());
        iter++;
    }
    //qDebug()<<"lists:"<<lists;
    this->addHeader(lists);
    //QString url = "http://"+options[HOST]+"/"+this->escape(options[OBJECT]);
    QString url = "http://"+options[HOST];
    if(!options[OBJECT].startsWith("/")){
        url += QLatin1String("/");
    }
    OSSResponse* response = new  OSSResponse(this->id);
    this->get(url,response);
    //response->debug();
    response->setCommand(QLatin1String("GET /%1").arg(prefix));
    if(response->errorCode==0){
        response->params["dir"] = dir;
    }
    return response;
}

NetworkResponse* OSS::upload(Task* task)
{
    QMutexLocker locker(&mutex);
    QString local = task->local;
    QString remote = task->remote;
    if(remote.startsWith("/")){
        remote = remote.mid(1);
    }
    NetworkResponse *response =  new HttpResponse;
    QFileInfo fi(local);
    if(fi.exists()){
        task->file = new QFile(local);
        if(task->file->open(QIODevice::ReadOnly)){
            task->filesize = fi.size();
            task->readysize = 0l;
            HttpParams options;
            options[BUCKET] = m_bucket;
            options[METHOD] = "PUT";
            options[OBJECT] = remote;
            QMimeDatabase db;
            QMimeType mime = db.mimeTypeForFile(local);
            QString fileMimeType = mime.name();
            options[CONTENT_TYPE] = fileMimeType;
            QStringList arr = this->host.split(".");
            QString host = this->host;
            options[HOST] = host;
            HttpParams headers = this->signHeaders(options);
            QStringList lists;
            HttpParamIter iter = headers.begin();
            while(iter!=headers.end()){
                lists.push_back((iter.key()+": "+iter.value()).toStdString().c_str());
                iter++;
            }
            lists.push_back("Expect:");
            this->addHeader(lists);
            //qDebug()<<"header:"<<lists;
            //QString url = "http://"+options[HOST]+"/"+this->escape(options[OBJECT]);
            QString url = "http://"+options[HOST];
            if(!options[OBJECT].startsWith("/")){
                url += QLatin1String("/");
            }
            url += this->escape(options[OBJECT]);
            OSSResponse* response = new  OSSResponse(this->id);
            this->setOption(CURLOPT_URL,url.toStdString().c_str());
            this->setOption(CURLOPT_POST,0);
            this->setOption(CURLOPT_HTTPPOST,nullptr);
            this->setOption(CURLOPT_CUSTOMREQUEST,"PUT");


            this->setOption(CURLOPT_READDATA,(void*)task);
            this->setOption(CURLOPT_READFUNCTION,network_read_callback);
            this->setOption(CURLOPT_UPLOAD,1L);
            this->setOption(CURLOPT_INFILESIZE_LARGE,(curl_off_t)(task->filesize));

            int size = m_requestHeaders.size();
            struct curl_slist *chunk = nullptr;
            if(size>0){
                for(auto header:m_requestHeaders){
                    chunk = curl_slist_append(chunk, header.toUtf8().constData());
                }
                this->setOption(CURLOPT_HTTPHEADER,chunk);
            }
            this->access(response);
            if(size>0){
                m_requestHeaders.clear();
                curl_slist_free_all(chunk);
            }

            //reset start
            this->setOption(CURLOPT_READDATA,nullptr);
            this->setOption(CURLOPT_READFUNCTION,nullptr);
            this->setOption(CURLOPT_UPLOAD,0);
            this->setOption(CURLOPT_INFILESIZE_LARGE,0);
            this->setOption(CURLOPT_HTTPHEADER,nullptr);
            //reset end

            //response->parse();
            //response->debug();
            task->file->close();
            delete task->file;
            task->file = nullptr;


            if(task->abort==true){
                response->errorCode = -3;
                response->errorMsg = QObject::tr("User aborted");
                task->abort = false;
            }
            response->setCommand(QLatin1String("PUT %1").arg(options[OBJECT]));
            return response;
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

NetworkResponse* OSS::download(Task* task)
{
    QMutexLocker locker(&mutex);
    //qDebug()<<"oss download";
    QString local = task->local;
    QString remote = task->remote;
    if(remote.startsWith("/")){
        remote = remote.mid(1);
    }
    NetworkResponse *response =  new HttpResponse;
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

            HttpParams options;
            options[BUCKET] = m_bucket;
            options[METHOD] = "GET";
            options[OBJECT] = remote;
            QStringList arr = this->host.split(".");
            QString host = this->host;
            options[HOST] = host;
            HttpParams headers = this->signHeaders(options);
            if(!headers.contains(ACCEPT_ENCODING)){
                headers[ACCEPT_ENCODING] = "";
            }
            QStringList lists;
            HttpParamIter iter = headers.begin();
            while(iter!=headers.end()){
                lists.push_back((iter.key()+": "+iter.value()).toStdString().c_str());
                iter++;
            }
            this->addHeader(lists);

            //QString url = "http://"+options[HOST]+"/"+this->escape(options[OBJECT]);
            QString url = "http://"+options[HOST];
            if(!options[OBJECT].startsWith("/")){
                url += QLatin1String("/");
            }
            url += this->escape(options[OBJECT]);
            OSSResponse* response = new  OSSResponse(this->id);


            this->setOption(CURLOPT_URL,url.toStdString().c_str());
            this->setOption(CURLOPT_POST,0);
            this->setOption(CURLOPT_HTTPPOST,nullptr);
            this->setOption(CURLOPT_CUSTOMREQUEST,"GET");

            this->setOption(CURLOPT_WRITEDATA, (void*)task->file);
            this->setOption(CURLOPT_WRITEFUNCTION, network_write_callback);
            this->setOption(CURLOPT_XFERINFOFUNCTION, network_progress_callback);
            this->setOption(CURLOPT_XFERINFODATA, (void*)task);
            this->setOption(CURLOPT_NOPROGRESS, 0L);
            this->setOption(CURLOPT_TIMEOUT, 0);

            int size = m_requestHeaders.size();
            struct curl_slist *chunk = nullptr;
            if(size>0){
                for(auto header:m_requestHeaders){
                    chunk = curl_slist_append(chunk, header.toUtf8().constData());
                }
                this->setOption(CURLOPT_HTTPHEADER,chunk);
            }
            this->access(response,false);
            if(size>0){
                m_requestHeaders.clear();
                curl_slist_free_all(chunk);
            }
            ///response->debug();
            //response->parse();
            task->file->close();
            delete task->file;
            task->file = nullptr;
            this->setOption(CURLOPT_WRITEDATA, nullptr);
            this->setOption(CURLOPT_WRITEFUNCTION, nullptr);
            this->setOption(CURLOPT_XFERINFOFUNCTION, nullptr);
            this->setOption(CURLOPT_XFERINFODATA, nullptr);
            this->setOption(CURLOPT_NOPROGRESS, 1L);
            this->setOption(CURLOPT_HTTPHEADER,nullptr);

            if(task->abort==true){
                response->errorCode = -3;
                response->errorMsg = QObject::tr("User aborted");
                task->abort = false;
            }
            response->setCommand(QLatin1String("GET %1").arg(options[OBJECT]));
            return response;
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

NetworkResponse* OSS::del(Task* task)
{
    if(task->type==0){
        QMutexLocker locker(&mutex);
        return nullptr;
    }else{
        return this->rmDir(task->remote);
    }
}

NetworkResponse* OSS::del(const QString& dst){
    return this->del({},dst);
}


NetworkResponse* OSS::del(const QString& bucket,const QString& dst)
{
    QMutexLocker locker(&mutex);
    HttpParams options;
    if(bucket.isEmpty()){
        options[BUCKET] = m_bucket;
    }else{
        options[BUCKET] = bucket;
    }
    options[METHOD] = "DELETE";
    options[OBJECT] = dst;
    QStringList arr = this->host.split(".");
    options[HOST] = this->host;
    HttpParams headers = this->signHeaders(options);

    QStringList lists;
    HttpParamIter iter = headers.begin();
    while(iter!=headers.end()){
        lists.push_back((iter.key()+": "+iter.value()).toStdString().c_str());
        iter++;
    }
    this->addHeader(lists);

    int size = m_requestHeaders.size();
    struct curl_slist *chunk = nullptr;
    if(size>0){
        for(auto header:m_requestHeaders){
            chunk = curl_slist_append(chunk, header.toUtf8().constData());
        }
        this->setOption(CURLOPT_HTTPHEADER,chunk);
    }

    OSSResponse* response = new  OSSResponse(this->id);

    //curl_easy_setopt(this->curl, CURLOPT_HEADERDATA, (void *)&response->header);
    //curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, (void *)&response->body);
    curl_easy_setopt(this->curl, CURLOPT_HTTPHEADER, chunk);
    //QString url = "http://"+options[HOST]+"/"+this->escape(options[OBJECT]);
    QString url = "http://"+options[HOST];
    if(!options[OBJECT].startsWith("/")){
        url += QLatin1String("/");
    }
    url += this->escape(options[OBJECT]);

    curl_easy_setopt(this->curl,CURLOPT_URL,url.toStdString().c_str());
    curl_easy_setopt(this->curl,CURLOPT_CUSTOMREQUEST,"DELETE");

    this->access(response,false);
    if(size>0){
        m_requestHeaders.clear();
        curl_slist_free_all(chunk);
    }
    response->setCommand(QLatin1String("DELETE %1").arg(options[OBJECT]));
    return response;
}



NetworkResponse* OSS::mkDir(const QString &dir)
{
    QMutexLocker locker(&mutex);
    HttpParams options;
    options[BUCKET] = m_bucket;
    options[METHOD] = "PUT";
    options[OBJECT] = dir;
    if(!options[OBJECT].endsWith("/")){
        options[OBJECT] += "/";
    }
    /*QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(local);
    QString fileMimeType = mime.name();
    options[CONTENT_TYPE] = fileMimeType;
    QStringList arr = this->host.split(".");*/
    QString host = this->host;
    options[HOST] = host;
    HttpParams headers = this->signHeaders(options);
    headers[CONTENT_LENGTH] = QString("%1").arg(0);
    QStringList lists;
    HttpParamIter iter = headers.begin();
    while(iter!=headers.end()){
        lists.push_back((iter.key()+": "+iter.value()).toStdString().c_str());
        iter++;
    }
    this->addHeader(lists);
    //QString url = "http://"+options[HOST]+"/"+this->escape(options[OBJECT]);
    QString url = "http://"+options[HOST];
    if(!options[OBJECT].startsWith("/")){
        url += QLatin1String("/");
    }
    url += this->escape(options[OBJECT]);

    OSSResponse* response = new  OSSResponse(this->id);
    this->setOption(CURLOPT_URL,url.toStdString().c_str());
    this->setOption(CURLOPT_POST,0);
    this->setOption(CURLOPT_HTTPPOST,nullptr);
    this->setOption(CURLOPT_CUSTOMREQUEST,"PUT");
    this->setOption(CURLOPT_INFILESIZE_LARGE,0);
    int size = m_requestHeaders.size();
    struct curl_slist *chunk = nullptr;
    if(size>0){
        for(auto header:m_requestHeaders){
            chunk = curl_slist_append(chunk, header.toUtf8().constData());
        }
        this->setOption(CURLOPT_HTTPHEADER,chunk);
    }
    this->access(response);
    if(size>0){
        m_requestHeaders.clear();
        curl_slist_free_all(chunk);
    }
    response->setCommand(QLatin1String("PUT %1").arg(options[OBJECT]));
    return response;
}

NetworkResponse* OSS::rmDir(const QString &dir)
{
    QString object = dir;
    if(!object.endsWith("/")){
        object += "/";
    }
    return del(m_bucket,object);
}



NetworkResponse* OSS::customeAccess(const QString& name,QMap<QString,QVariant> data)
{
    //return this->errorCode;
    return nullptr;
}

static bool orMatches(const QList<QRegularExpression> &exprList, const QString &filePath)
{
    bool ret = false;
    for(auto reg:exprList){
        ret = reg.match(filePath).hasMatch();
        if(ret){
            break;
        }
    }
    return ret;
}


QString OSS::matchToPath(const QString& from,bool local){
    if(local && from.endsWith("/")==false){
        //file
        if(m_filters.size()>0 && orMatches(m_filters,from)==false){
            return {};
        }
    }
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


HttpParams OSS::signHeaders(const HttpParams& options)
{
    time_t rawtime;
    struct tm * ptm;
    time ( &rawtime );
    ptm = gmtime ( &rawtime );
    char datetime [80];
    ::setlocale(LC_TIME,"C");
    strftime (datetime,80,"%a, %d %b %G %T GMT",ptm);


    HttpParams headers;
    headers[CONTENT_MD5] = "";
    headers[CONTENT_TYPE] = "";
    headers[DATE] = datetime;
    headers[HOST] = options[HOST];
    if(options.contains(METHOD)){
        if(!options.contains(CONTENT_TYPE)){
            headers[CONTENT_TYPE] = "application/octet-stream";
        }else{
             headers[CONTENT_TYPE] = options[CONTENT_TYPE];
        }
    }
    QString string_to_sign;
    if(options.contains(METHOD)){
        string_to_sign += (options[METHOD] + "\n");
    }
    if(options.contains(CONTENT_MD5)){
        string_to_sign += (options[CONTENT_MD5] + "\n");
    }
    HttpParamIter iter = headers.begin();
    while(iter!=headers.end()){
        QString header_key = iter.key().toLower();
        if(header_key=="content-md5"||header_key=="content-type"||header_key=="date"){
            string_to_sign += (iter.value()+"\n");
        }
        iter++;
    }
    QString signableResource = "/";
    if(options.contains(BUCKET) && options[BUCKET].isEmpty()==false){
        signableResource += options[BUCKET];
        if(options.contains(OBJECT) && options[OBJECT]=="/"){
            signableResource += "/";
        }
    }

    if(options.contains(OBJECT) && options[OBJECT]!="/"){
        if(!options[OBJECT].startsWith("/")){
            signableResource += "/";
        }
        signableResource += this->escape(options[OBJECT]);
    }

    //GET\n\napplication/octet-stream\nWed, 29 Mar 2023 02:08:48 GMT\n
    //GET\n\napplication/octet-stream\nWed, 29 Mar 2023 02:08:48 GMT\n/qnsapp123/statics/
    string_to_sign += signableResource;
    //qDebug()<<"data:"<<options;
    //qDebug()<<"sign:"<<string_to_sign.toUtf8();
    HMAC_CTX *ctx = HMAC_CTX_new();
    HMAC_CTX_reset(ctx);
    unsigned int len = 20;
    int size = sizeof(char) * len;
    unsigned char* result = (unsigned char*) new char[size];

    HMAC_Init_ex(ctx, this->password.toStdString().c_str(), this->password.length(), EVP_sha1(), NULL);
    //HMAC_Update(ctx, (unsigned char*)string_to_sign.toStdString().c_str(),string_to_sign.length() );
    auto bytearr = string_to_sign.toUtf8();
    HMAC_Update(ctx, (unsigned char*)bytearr.data(),bytearr.length() );
    HMAC_Final(ctx, result, &len);
    HMAC_CTX_free(ctx);

    QByteArray by;
    by.append((const char *)result,size);
    QString Authorization = "OSS "+this->username+":"+by.toBase64();
    headers[AUTHORIZATION] = Authorization;
    delete result;
    return headers;
}

}
