#include "cos.h"
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QMimeDatabase>
#include <QCryptographicHash>
#include "cos_response.h"
#include "cos_list_object_response.h"
#include "transfer/Task.h"
#include <openssl/hmac.h>
#include <openssl/opensslconf.h>
#include <locale.h>
#include <common/utils.h>

#ifdef Q_OS_WIN
#include <hmac/hmac_local.h>
#endif


#include <QDebug>

namespace ady {
constexpr const  char COS::METHOD[] ;
constexpr const  char COS::OBJECT[] ;
constexpr const  char COS::BUCKET[] ;
constexpr const  char COS::CONTENT_TYPE[];
constexpr const  char COS::CONTENT_MD5[] ;
constexpr const  char COS::DATE[] ;
constexpr const  char COS::HOST[] ;
constexpr const  char COS::CONTENT_LENGTH[] ;
constexpr const  char COS::AUTHORIZATION[] ;
constexpr const  char COS::DELIMITER[] ;
constexpr const  char COS::MARKER[] ;
constexpr const  char COS::MAXKEYS[] ;
constexpr const  char COS::PREFIX[] ;
constexpr const  char COS::ACCEPT_ENCODING[] ;

COS::COS(CURL* curl,long long id)
    :HttpClient(curl,id)
{
    this->connected = false;
    this->setOption(CURLOPT_TIMEOUT,COMMAND_TIMEOUT);
    this->m_setting = nullptr;
}

COS::~COS(){
    if(this->m_setting!=nullptr){
        delete m_setting;
    }
}

void COS::init(const SiteRecord& info){
    this->setHost(info.host);
    this->setPort(info.port);
    this->setUsername(info.username);
    this->setPassword(info.password);
    m_rootPath = info.path;

    if(m_setting!=nullptr){
        delete m_setting;
    }

    m_setting = new COSSetting(info.data);
    m_dirMapping = m_setting->dirMapping();

    m_filters.clear();
    for(auto filter:m_setting->filterExtensions()){
        m_filters << QRegularExpression(Utils::stringToExpression(filter));
    }
}




NetworkResponse* COS::link()
{
    QMutexLocker locker(&mutex);
    m_bucket = this->host.left(this->host.indexOf("."));
    int pos = m_bucket.lastIndexOf("-");
    if(pos>-1){
        m_bucket = m_bucket.left(pos);
    }
    QString dst = this->m_rootPath;
    HttpParams headers;
    HttpParams params;
    //headers[BUCKET] = m_bucket;
    //headers[METHOD] = "GET";
    //headers[OBJECT] = dst;
    //QStringList arr = this->host.split(".");
    QString host = this->host;
    headers[HOST] = host;


    QString prefix = this->m_rootPath;
    if(prefix.startsWith("/")){
        prefix = prefix.mid(1);
    }
    params[PREFIX] = prefix;
    params[MARKER] = "";
    params[DELIMITER] = "/";
    params[MAXKEYS] = "1";
    QString Authorization = this->signHeaders("GET","/",headers,params);
    headers[AUTHORIZATION] = Authorization;
    QStringList lists;
    HttpParamIter iter = headers.begin();
    while(iter!=headers.end()){
        lists.push_back((iter.key()+": "+iter.value()).toStdString().c_str());
        iter++;
    }
    this->addHeader(lists);
    this->setOption(CURLOPT_NOBODY,1);
    QString url = "http://"+headers[HOST]+"/";
    url = this->fixUrl(url,params);
   qDebug()<<"url:"<<url;
    COSResponse* response = new  COSResponse(this->id);
    this->get(url,response);
    this->setOption(CURLOPT_NOBODY,0);

    response->setCommand(QLatin1String("GET /%1").arg(prefix));
    return response;

}

NetworkResponse* COS::unlink()
{
    QMutexLocker locker(&mutex);
    return nullptr;
}

NetworkResponse* COS::listDir(const QString& dir,int page,int pageSize)
{
    Q_UNUSED(page);
    //Q_UNUSED(pageSize);
    QMutexLocker locker(&mutex);
    m_bucket = this->host.left(this->host.indexOf("."));
    QStringList arr = this->host.split(".");
    QString host = this->host;
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
    if(prefix=="/"){
        prefix = "";
    }
    QString marker = "";
    COSListObjectResponse* response = new COSListObjectResponse();
    do{
        HttpParams headers;
        HttpParams params;
        headers[HOST] = host;

        params[PREFIX] = prefix;
        params[MARKER] = marker;
        params[DELIMITER] = "/";
        params[MAXKEYS] = QString("%1").arg(pageSize);

        QString Authorization = this->signHeaders("GET","/",headers,params);
        headers[AUTHORIZATION] = Authorization;

        QStringList lists;
        HttpParamIter iter = headers.begin();
        while(iter!=headers.end()){
            lists.push_back((iter.key()+": "+iter.value()).toStdString().c_str());
            iter++;
        }
        this->addHeader(lists);
        QString url = "http://"+headers[HOST]+"/";
        url = this->fixUrl(url,params);
        COSResponse* res = new  COSResponse(this->id);
        this->get(url,res);
        //res->debug();
        response->id = res->id;
        response->errorCode = res->errorCode;
        response->networkErrorCode = res->networkErrorCode;
        if(res->status()){
            response->setStatus(true);
            response->appendLists(res->parseList());
            marker = res->marker();
            response->params = res->params;
            response->header = res->header;
            delete res;
            if(marker.isEmpty()){
                break;
            }
        }else{
            response->setStatus(false);
            response->errorMsg = res->errorMsg;
            response->networkErrorMsg = res->networkErrorMsg;
            delete res;
            break;
        }
    }while(true);

    response->setCommand(QLatin1String("GET /%1").arg(prefix));
    if(response->errorCode==0){
        response->params["dir"] = dir;
    }
    return response;

}

NetworkResponse* COS::tinyListDir(const QString& dir)
{

    QMutexLocker locker(&mutex);
    m_bucket = this->host.left(this->host.indexOf("."));
    QStringList arr = this->host.split(".");
    QString host = this->host;
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
    if(prefix=="/"){
        prefix = "";
    }
    QString marker = "";
    COSListObjectResponse* response = new COSListObjectResponse();
    do{
        HttpParams headers;
        HttpParams params;
        headers[HOST] = host;

        params[PREFIX] = prefix;
        params[MARKER] = marker;
        params[DELIMITER] = "/";
        params[MAXKEYS] = "1000";

        QString Authorization = this->signHeaders("GET","/",headers,params);
        headers[AUTHORIZATION] = Authorization;

        QStringList lists;
        HttpParamIter iter = headers.begin();
        while(iter!=headers.end()){
            lists.push_back((iter.key()+": "+iter.value()).toStdString().c_str());
            iter++;
        }
        this->addHeader(lists);
        QString url = "http://"+headers[HOST]+"/";
        url = this->fixUrl(url,params);
        COSResponse* res = new  COSResponse(this->id);
        this->get(url,res);

        response->errorCode = res->errorCode;
        response->networkErrorCode = res->networkErrorCode;
        if(res->status()){
            response->setStatus(true);
            response->appendLists(res->parseList());
            marker = res->marker();
            response->params = res->params;
            response->header = res->header;
            delete res;
            if(marker.isEmpty()){
                break;
            }
        }else{
            response->errorMsg = res->errorMsg;
            response->networkErrorMsg = res->networkErrorMsg;
            response->setStatus(false);
            response->errorMsg = res->errorMsg;
            delete res;
            break;
        }
    }while(true);

    //response->setCommand("GET "+prefix);
    response->setCommand(QLatin1String("GET /%1").arg(prefix));
    if(response->errorCode==0){
        response->params["dir"] = dir;
    }
    return response;
}

NetworkResponse* COS::upload(Task* task)
{
    QMutexLocker locker(&mutex);
    QString local = task->local;
    QString remote = task->remote;
    NetworkResponse *response =  new HttpResponse;
    QFileInfo fi(local);
    if(fi.exists()){
        task->file = new QFile(local);
        if(task->file->open(QIODevice::ReadOnly)){

            task->filesize = fi.size();
            task->readysize = 0l;
            HttpParams headers;
            HttpParams params;
            QMimeDatabase db;
            QMimeType mime = db.mimeTypeForFile(local);
            QString fileMimeType = mime.name();
            headers[CONTENT_TYPE] = fileMimeType;
            //headers[CONTENT_MD5] = md5;
            headers[CONTENT_LENGTH] = QString("%1").arg(task->filesize);
            QStringList arr = this->host.split(".");
            QString host = this->host;
            headers[HOST] = host;
            QString Authorization = this->signHeaders("PUT",remote,headers,params);
            headers[AUTHORIZATION] = Authorization;
            QStringList lists;
            HttpParamIter iter = headers.begin();
            while(iter!=headers.end()){
                lists.push_back((iter.key()+": "+iter.value()).toStdString().c_str());
                iter++;
            }
            lists.push_back("Expect:");
            this->addHeader(lists);
            QString url = "http://"+headers[HOST];
            if(!remote.startsWith("/")){
                url += QLatin1String("/");
            }
            url += this->escape(remote);

            COSResponse* response = new  COSResponse(this->id);
            this->setOption(CURLOPT_URL,url.toStdString().c_str());
            this->setOption(CURLOPT_POST,0);
            this->setOption(CURLOPT_HTTPPOST,nullptr);
            this->setOption(CURLOPT_CUSTOMREQUEST,"PUT");



            this->setOption(CURLOPT_TIMEOUT,(std::max)((int)task->filesize/1024*20,3));


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

            task->file->close();
            delete task->file;
            task->file = nullptr;

            if(task->abort==true){
                response->errorCode = -3;
                response->errorMsg = QObject::tr("User aborted");
                task->abort = false;
            }

            response->setCommand(QLatin1String("PUT %1").arg(remote));


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

NetworkResponse* COS::download(Task* task)
{
    QMutexLocker locker(&mutex);
    //qDebug()<<"COS download";
    QString local = task->local;
    QString remote = task->remote;
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

            HttpParams headers;
            HttpParams params;
            QStringList arr = this->host.split(".");
            QString host = this->host;
            headers[HOST] = host;
            QString Authorization = this->signHeaders("GET",remote,headers,params);
            headers[AUTHORIZATION] = Authorization;
            QStringList lists;
            HttpParamIter iter = headers.begin();
            while(iter!=headers.end()){
                lists.push_back((iter.key()+": "+iter.value()).toStdString().c_str());
                iter++;
            }
            this->addHeader(lists);

            QString url = "http://"+headers[HOST];
            if(!remote.startsWith("/")){
                url += QLatin1String("/");
            }
            url += this->escape(remote);

            COSResponse* response = new  COSResponse(this->id);
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

            response->setCommand(QLatin1String("GET %1").arg(remote));

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

NetworkResponse* COS::del(Task* task)
{
    if(task->type==0){
        QMutexLocker locker(&mutex);
        return nullptr;
    }else{
        return this->rmDir(task->remote);
    }
}

NetworkResponse* COS::del(const QString &dst){
    return this->del({},dst);
}

NetworkResponse* COS::del(const QString& bucket,const QString& dst)
{
    QMutexLocker locker(&mutex);
    HttpParams headers;
    HttpParams params;
    QString path = dst;
    if(path.startsWith("/")){
        path = dst.mid(1);
    }
    QStringList arr = this->host.split(".");
    headers[HOST] = this->host;
    QString Authorization = this->signHeaders("DELETE","/"+path,headers,params);
    headers[AUTHORIZATION] = Authorization;

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

    COSResponse* response = new  COSResponse(this->id);
    curl_easy_setopt(this->curl, CURLOPT_HTTPHEADER, chunk);
    QString url = "http://"+headers[HOST];
    if(!dst.startsWith("/")){
        url += QLatin1String("/");
    }
    url += this->escape(dst);
    curl_easy_setopt(this->curl,CURLOPT_URL,url.toStdString().c_str());
    curl_easy_setopt(this->curl,CURLOPT_CUSTOMREQUEST,"DELETE");

    this->access(response,false);
    if(size>0){
        m_requestHeaders.clear();
        curl_slist_free_all(chunk);
    }
    response->setCommand(QLatin1String("DELETE %1").arg(dst));

    return response;
}



NetworkResponse* COS::mkDir(const QString &dir)
{
    QMutexLocker locker(&mutex);
    HttpParams headers;
    HttpParams params;
    /*headers[BUCKET] = m_bucket;
    headers[METHOD] = "PUT";
    headers[OBJECT] = dir;
    if(!headers[OBJECT].endsWith("/")){
        headers[OBJECT] += "/";
    }*/
    QString object = dir;
    if(!object.endsWith("/")){
        object += "/";
    }
    //qDebug()<<"object:"<<object;
    QString host = this->host;
    headers[HOST] = host;
    QString Authorization = this->signHeaders("PUT",object,headers,params);
    headers[AUTHORIZATION] = Authorization;
    headers[CONTENT_LENGTH] = QString("%1").arg(0);
    QStringList lists;
    HttpParamIter iter = headers.begin();
    while(iter!=headers.end()){
        lists.push_back((iter.key()+": "+iter.value()).toStdString().c_str());
        iter++;
    }
    //lists.push_back("Expect:");
    this->addHeader(lists);
    QString url = "http://"+headers[HOST]+this->escape(object);
    //qDebug()<<"url:"<<url;
    COSResponse* response = new  COSResponse(this->id);
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
    //qDebug()<<"lists:"<<lists;
    response->debug();
    //response->parse();
    return response;
}

NetworkResponse* COS::rmDir(const QString &dir)
{
    QString object = dir;
    if(!object.endsWith("/")){
        object += "/";
    }
    return del(m_bucket,object);
}




NetworkResponse* COS::customeAccess(const QString& name,QMap<QString,QVariant> data)
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

QString COS::matchToPath(const QString& from,bool local){
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

QString COS::fixUrl(QString& url,HttpParams& params)
{
    if(params.size()<=0){
        return url;
    }
    HttpParamIter iter = params.begin();
    if(url.indexOf("?")>0){
        if(!url.endsWith("&")){
            url +="&";
        }
    }else{
        url += "?";
    }
    while(iter!=params.end()){
        url += (iter.key()+"="+urlEncode(iter.value())+"&");
        iter++;
    }
    if(url.endsWith("&")){
        url = url.left(url.length()-1);
    }
    return url;
}

QString COS::signHeaders(QString method,QString path, HttpParams& headers, HttpParams& params)
{
    time_t rawtime;
    struct tm * ptm;
    time ( &rawtime );
    ptm = gmtime ( &rawtime );
    char datetime [80];
    ::setlocale(LC_TIME,"C");
    strftime (datetime,80,"%a, %d %b %G %T GMT",ptm);

    QString headerList;
    QString httpHeaders;
    HttpParamIter iter = headers.begin();
    while(iter!=headers.end()){
        headerList += (urlEncode(iter.key()).toLower() + ";");
        httpHeaders += (urlEncode(iter.key()).toLower() + "=" + urlEncode(iter.value()) + "&");
        iter++;
    }
    if(headerList.endsWith(";")){
        headerList = headerList.left(headerList.length() - 1);
    }
    if(httpHeaders.endsWith("&")){
        httpHeaders = httpHeaders.left(httpHeaders.length() - 1);
    }

    QString paramList;
    QString httpParams;
    iter = params.begin();
    while(iter!=params.end()){
        paramList += (urlEncode(iter.key()).toLower() + ";");
        httpParams += (urlEncode(iter.key()).toLower() + "=" + urlEncode(iter.value()) + "&");
        iter++;
    }
    if(paramList.endsWith(";")){
        paramList = paramList.left(paramList.length() - 1);
    }
    if(httpParams.endsWith("&")){
        httpParams = httpParams.left(httpParams.length() - 1);
    }

    //qDebug()<<"httpHeaders:"<<httpHeaders;
    //qDebug()<<"httpParams:"<<httpParams;


    time_t tm = time(NULL);
    std::string keyTime = std::to_string(tm)+";"+std::to_string(tm + 86400);

    QString httpString = method.toLower() + "\n" + (path) + "\n" + (httpParams) + "\n" + httpHeaders + "\n";

    //QCryptographicHash hash(QCryptographicHash::Sha1);
    //hash.addData(httpString.toStdString().c_str(),httpString.length());
    QString str = QCryptographicHash::hash(httpString.toStdString().c_str(),QCryptographicHash::Sha1).toHex();




    qDebug()<<"httpString:"<<httpString;
    qDebug()<<"sha1:"<<str;

    QString stringToSign = QString("sha1\n%1\n").arg(keyTime.c_str());
    stringToSign +=str;
    stringToSign += "\n";
    qDebug()<<"stringToSign:"<<stringToSign;

    QByteArray byte;
    QString signKey;

    {
        HMAC_CTX *ctx = HMAC_CTX_new();
        HMAC_CTX_reset(ctx);
        unsigned int len = 20;
        int size = sizeof(char) * len;
        unsigned char* result = (unsigned char*) new char[size];

        HMAC_Init_ex(ctx, this->password.toStdString().c_str(), this->password.length(), EVP_sha1(), NULL);
        HMAC_Update(ctx, (unsigned char*)keyTime.c_str(),keyTime.length() );
        HMAC_Final(ctx, result, &len);
        HMAC_CTX_free(ctx);

        byte.append((const char *)result,len);


        signKey =byte.toHex();
        //qDebug()<<"password:"<<password;
        //qDebug()<<"signKey:"<<signKey;
        delete result;
    }


    HMAC_CTX *ctx = HMAC_CTX_new();
    HMAC_CTX_reset(ctx);
    unsigned int len = 20;
    int size = sizeof(char) * len;
    unsigned char* result = (unsigned char*) new char[size];

    HMAC_Init_ex(ctx, signKey.toStdString().c_str(), signKey.length(), EVP_sha1(), NULL);
    HMAC_Update(ctx, (unsigned char*)stringToSign.toStdString().c_str(),stringToSign.length() );
    HMAC_Final(ctx, result, &len);
    HMAC_CTX_free(ctx);

    QByteArray by;
    by.append((const char *)result,size);
    QString signature = by.toHex();
    QString Authorization = QString("q-sign-algorithm=sha1&q-ak=%1&q-sign-time=%2&q-key-time=%3&q-header-list=%4&q-url-param-list=%5&q-signature=%6")
            .arg(this->username)
            .arg(keyTime.c_str())
            .arg(keyTime.c_str())
            .arg(headerList)
            .arg(paramList)
            .arg(signature);
    delete result;
    qDebug()<<"Authorization:"<<Authorization;
    return Authorization;

}

QString COS::urlEncode(const QString& text)
{
    QString strTemp = "";
    std::string str = text.toStdString();
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ')
            strTemp += "%20";
        else
        {
            strTemp += '%';
            strTemp += toHex((unsigned char)str[i] >> 4);
            strTemp += toHex((unsigned char)str[i] % 16);
        }
    }
    return strTemp;
}

}
