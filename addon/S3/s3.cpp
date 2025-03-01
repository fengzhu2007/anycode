#include "s3.h"
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QTimeZone>
#include <QMimeDatabase>
#include <QCryptographicHash>
#include "s3_response.h"
#include "s3_list_object_response.h"
#include "transfer/task.h"
#include <openssl/hmac.h>
#include <openssl/opensslconf.h>
#include <locale.h>
#include <common/utils.h>

#ifdef Q_OS_WIN
#include <hmac/hmac_local.h>
#endif
#include "s3_setting.h"


#include <QDebug>

namespace ady {
constexpr const  char S3::METHOD[] ;
constexpr const  char S3::OBJECT[] ;
constexpr const  char S3::BUCKET[] ;
constexpr const  char S3::CONTENT_TYPE[];
constexpr const  char S3::CONTENT_MD5[] ;
constexpr const  char S3::DATE[] ;
constexpr const  char S3::HOST[] ;
constexpr const  char S3::CONTENT_LENGTH[] ;
constexpr const  char S3::AUTHORIZATION[] ;
constexpr const  char S3::DELIMITER[] ;
constexpr const  char S3::CONTINUATION_TOKEN[] ;
constexpr const  char S3::MAXKEYS[] ;
constexpr const  char S3::PREFIX[] ;
constexpr const  char S3::ACCEPT_ENCODING[] ;
constexpr const  char S3::LIST_TYPE[] ;

static QString nameEncode(const QString& str){
    return QUrl::toPercentEncoding(str,"/");
}

S3::S3(CURL* curl,long long id)
    :HttpClient(curl,id)
{
    this->connected = false;
    this->setOption(CURLOPT_TIMEOUT,COMMAND_TIMEOUT);
    this->m_setting = nullptr;
}

S3::~S3(){
    if(this->m_setting!=nullptr){
        delete m_setting;
    }
}

void S3::init(const SiteRecord& info){
    this->setHost(info.host);
    this->setPort(info.port);
    this->setUsername(info.username);
    this->setPassword(info.password);
    m_rootPath = info.path;

    if(m_setting!=nullptr){
        delete m_setting;
    }

    m_setting = new S3Setting(info.data);
    m_dirMapping = m_setting->dirMapping();

    m_filters.clear();
    for(auto filter:m_setting->filterExtensions()){
        m_filters << QRegularExpression(Utils::stringToExpression(filter));
    }
}

NetworkResponse* S3::link()
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
    QString host = this->host;
    headers[HOST] = host;



    QString prefix = this->m_rootPath;
    if(prefix.startsWith("/")){
        prefix = prefix.mid(1);
    }
    params[PREFIX] = prefix;
    params[DELIMITER] = "/";
    params[MAXKEYS] = "1";
    params[LIST_TYPE] = "2";
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
    S3Response* response = new  S3Response(this->id);
    this->get(url,response);
    this->setOption(CURLOPT_NOBODY,0);
    response->debug();

    response->setCommand(QLatin1String("GET /%1").arg(prefix));
    return response;

}

NetworkResponse* S3::unlink()
{
    QMutexLocker locker(&mutex);
    return nullptr;
}

NetworkResponse* S3::listDir(const QString& dir,int page,int pageSize)
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
    QString continuation_token = "";
    S3ListObjectResponse* response = new S3ListObjectResponse();
    do{
        HttpParams headers;
        HttpParams params;
        headers[HOST] = host;

        params[PREFIX] = prefix;
        //params[MARKER] = marker;
        if(!continuation_token.isEmpty()){
            params[CONTINUATION_TOKEN] = continuation_token;
        }
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
        S3Response* res = new  S3Response(this->id);
        this->get(url,res);
        res->debug();
        response->id = res->id;
        response->errorCode = res->errorCode;
        response->networkErrorCode = res->networkErrorCode;
        if(res->status()){
            response->setStatus(true);
            response->appendLists(res->parseList());
            continuation_token = res->continuationToken();
            response->params = res->params;
            response->header = res->header;
            delete res;
            if(continuation_token.isEmpty()){
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

NetworkResponse* S3::tinyListDir(const QString& dir)
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
    QString continuation_token = "";
    S3ListObjectResponse* response = new S3ListObjectResponse();
    do{
        HttpParams headers;
        HttpParams params;
        headers[HOST] = host;

        params[PREFIX] = prefix;
        if(!continuation_token.isEmpty()){
            params[CONTINUATION_TOKEN] = continuation_token;
        }
        //params[MARKER] = marker;
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
        S3Response* res = new  S3Response(this->id);
        this->get(url,res);

        response->errorCode = res->errorCode;
        response->networkErrorCode = res->networkErrorCode;
        if(res->status()){
            response->setStatus(true);
            response->appendLists(res->parseList());
            continuation_token = res->continuationToken();
            response->params = res->params;
            response->header = res->header;
            delete res;
            if(continuation_token.isEmpty()){
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

NetworkResponse* S3::upload(Task* task)
{
    QMutexLocker locker(&mutex);
    QString local = task->local;
    QString remote = nameEncode(task->remote);
    NetworkResponse *response =  new HttpResponse;
    QFileInfo fi(local);
    if(fi.exists()){
        task->file = new QFile(local);
        if(task->file->open(QIODevice::ReadOnly)){
            QByteArray buffer;
            QCryptographicHash hash(QCryptographicHash::Sha256);
            while (!task->file->atEnd()) {
                buffer = task->file->read(64 * 1024);
                hash.addData(buffer);
            }
            task->file->seek(0);



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
            QString Authorization = this->signHeaders("PUT",remote,headers,params,hash.result().toHex());
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
            url += (remote);

            S3Response* response = new  S3Response(this->id);
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

NetworkResponse* S3::download(Task* task)
{
    QMutexLocker locker(&mutex);
    //qDebug()<<"S3 download";
    QString local = task->local;
    QString remote = nameEncode(task->remote);
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

            S3Response* response = new  S3Response(this->id);
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

NetworkResponse* S3::del(Task* task)
{
    if(task->type==0){
        QMutexLocker locker(&mutex);
        return nullptr;
    }else{
        return this->rmDir(task->remote);
    }
}

NetworkResponse* S3::del(const QString &dst){
    return this->del({},dst);
}

NetworkResponse* S3::del(const QString& bucket,const QString& dst)
{
    QMutexLocker locker(&mutex);
    HttpParams headers;
    HttpParams params;
    QString path = dst;
    if(path.startsWith("/")){
        path = dst.mid(1);
    }
    path = nameEncode(path);
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

    S3Response* response = new  S3Response(this->id);
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



NetworkResponse* S3::mkDir(const QString &dir)
{
    QMutexLocker locker(&mutex);
    HttpParams headers;
    HttpParams params;
    QString object = dir;
    if(!object.endsWith("/")){
        object += "/";
    }
    object = nameEncode(object);
    qDebug()<<"object:"<<object;
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
    this->addHeader(lists);
    QString url = "http://"+headers[HOST]+object;
    S3Response* response = new  S3Response(this->id);
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
    //response->debug();
    return response;
}

NetworkResponse* S3::rmDir(const QString &dir)
{
    QString object = dir;
    if(!object.endsWith("/")){
        object += "/";
    }
    return del(m_bucket,object);
}




NetworkResponse* S3::customeAccess(const QString& name,QMap<QString,QVariant> data)
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

QString S3::matchToPath(const QString& from,bool is_file,bool local){
    //qDebug()<<"from"<<from<<local;
    if(local && is_file){
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
            qDebug()<<"from"<<from;
            for(auto one:m_dirMapping){
                const QString localPath =  one.first.startsWith("/")?one.first.mid(1):one.first;//like  path1/path2/
                const QString remotePath = one.second.startsWith("/")?one.second.mid(1):one.second;//like path3/path4/
                qDebug()<<"localPath"<<localPath<<remotePath;
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

QString S3::fixUrl(QString& url,HttpParams& params)
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


static QByteArray hash_hmac_sha256(const QByteArray& bytes,const QByteArray& key){
    HMAC_CTX *ctx = HMAC_CTX_new();
    HMAC_CTX_reset(ctx);
    unsigned int len = 32;
    int size = sizeof(char) * len;
    unsigned char* result = (unsigned char*) new char[size];

    HMAC_Init_ex(ctx, key.data(), key.length(), EVP_sha256(), NULL);
    HMAC_Update(ctx, (unsigned char*)bytes.data(),bytes.length() );
    HMAC_Final(ctx, result, &len);
    HMAC_CTX_free(ctx);
    QByteArray by;
    by.append((const char *)result,len);
    delete result;
    return by;
}

QString S3::signHeaders(QString method,QString path, HttpParams& headers, HttpParams& params,const QString& hashPayload)
{
    QDateTime now = QDateTime::currentDateTimeUtc();
    //now.setTimeZone(QTimeZone::utc());
    QString dateTime = now.toString("yyyyMMddThhmmssZ");
    headers.insert(S3::DATE,dateTime);
    headers.insert(S3::CONTENT_SHA256,hashPayload);
    QString headerList;
    QString httpHeaders;
    HttpParamIter iter = headers.begin();
    while(iter!=headers.end()){
        headerList += ((iter.key()).toLower() + ";");
        httpHeaders += ((iter.key()).toLower() + ":" + (iter.value()) + "\n");
        iter++;
    }
    //QString canonicalizedHeaderNames = headerList;
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
    if(httpParams.endsWith("&")){
        httpParams = httpParams.left(httpParams.length() - 1);
    }

    QString regionName="us-east-1";//default us-east-1
    QStringList list = this->host.split(".");//host bucket-name.s3.region-name.amazonaws.com
    if(list.size()==5){
        regionName = list.at(2);
    }

    const QString serviceName="s3";
    const QString TERMINATOR = "aws4_request";
    const QString SCHEME = "AWS4";
    const QString ALGORITHM = "HMAC-SHA256";


    QString dateStamp = now.toString("yyyyMMdd");
    QString scope =  dateStamp + "/" + regionName + "/" + serviceName + "/" + TERMINATOR;

    QString canonicalRequest = method + "\n"+path + "\n"+httpParams+"\n"+httpHeaders+"\n"+headerList+"\n"+hashPayload;

    QString stringToSign = SCHEME + "-" + ALGORITHM + "\n" + dateTime + "\n" + scope + "\n" + QCryptographicHash::hash(canonicalRequest.toUtf8(),QCryptographicHash::Sha256).toHex();
    //qDebug()<<"hashPayload"<<hashPayload;
    //qDebug()<<"canonicalRequest"<<canonicalRequest;
    QByteArray dateKey = hash_hmac_sha256(dateStamp.toUtf8(),(SCHEME+this->password).toUtf8());
    //qDebug()<<"dateKey"<<dateStamp<<SCHEME+this->password<<dateKey<<dateKey.toHex()<<"------------";
    QByteArray dateRegionKey = hash_hmac_sha256(regionName.toUtf8(),dateKey);
    QByteArray dateRegionServiceKey = hash_hmac_sha256(serviceName.toUtf8(),dateRegionKey);
    QByteArray signatureKey = hash_hmac_sha256(TERMINATOR.toUtf8(),dateRegionServiceKey);
    QByteArray signature = hash_hmac_sha256(stringToSign.toUtf8(),signatureKey);
    //qDebug()<<"Signature"<<signature.toHex();
    QString credentialsAuthorizationHeader =
        "Credential=" + this->username + "/" + scope;
    QString signedHeadersAuthorizationHeader =
        "SignedHeaders=" + headerList;
    QString signatureAuthorizationHeader =
        "Signature=" + signature.toHex();

    QString Authorization = SCHEME + "-" + ALGORITHM + " "
                                 + credentialsAuthorizationHeader + ", "
                                 + signedHeadersAuthorizationHeader + ", "
                                 + signatureAuthorizationHeader;
    return Authorization;

}

QString S3::urlEncode(const QString& text)
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
    //return QUrl::toPercentEncoding(text,"/");
}

}
