#include "cos.h"
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QMimeDatabase>
#include <QCryptographicHash>
#include "COSResponse.h"
#include "COSListObjectResponse.h"
#include "transfer/Task.h"
#include <openssl/hmac.h>
#include <openssl/opensslconf.h>
#include <locale.h>

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

COS::COS(CURL* curl)
    :HttpClient(curl)
{
    this->connected = false;
    this->setOption(CURLOPT_TIMEOUT,COMMAND_TIMEOUT);
}

/*int COS::access(NetworkResponse* response,bool body)
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
    response->errorCode = this->errorCode;
    response->errorMsg = this->errorMsg;
    response->parse();
    return (int)res;
}*/

NetworkResponse* COS::link()
{
    QMutexLocker locker(&mutex);
    m_bucket = this->host.left(this->host.indexOf("."));
    int pos = m_bucket.lastIndexOf("-");
    if(pos>-1){
        m_bucket = m_bucket.left(pos);
    }
    qDebug()<<"bucket:"<<m_bucket;
    QString dst = this->m_defaultDir;
    HttpParams headers;
    HttpParams params;
    //headers[BUCKET] = m_bucket;
    //headers[METHOD] = "GET";
    //headers[OBJECT] = dst;
    //QStringList arr = this->host.split(".");
    QString host = this->host;
    headers[HOST] = host;


    QString prefix = this->m_defaultDir;
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
   //qDebug()<<"url:"<<url;
    COSResponse* response = new  COSResponse();
    this->get(url,response);
    this->setOption(CURLOPT_NOBODY,0);
    return response;

}

NetworkResponse* COS::unlink()
{
    QMutexLocker locker(&mutex);
    return nullptr;
}

NetworkResponse* COS::listDir(QString dir,int page,int pageSize)
{
    Q_UNUSED(page);
    //Q_UNUSED(pageSize);
    QMutexLocker locker(&mutex);
    m_bucket = this->host.left(this->host.indexOf("."));
    QStringList arr = this->host.split(".");
    QString host = this->host;
    QString prefix = this->m_defaultDir;
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
        //qDebug()<<"url:"<<url;
        COSResponse* res = new  COSResponse();
        this->get(url,res);
        res->debug();
        if(res->status()){
            response->setStatus(true);
            response->appendLists(res->parseList());
            marker = res->marker();
            response->params = res->params;
            //qDebug()<<"marker:"<<marker;
            delete res;
            if(marker.isEmpty()){
                break;
            }
        }else{
            response->setStatus(false);
            response->errorMsg = res->errorMsg;
            delete res;
            break;
        }
    }while(true);
    return response;

}

NetworkResponse* COS::tinyListDir(QString dir)
{

    QMutexLocker locker(&mutex);
    m_bucket = this->host.left(this->host.indexOf("."));
    QStringList arr = this->host.split(".");
    QString host = this->host;
    QString prefix = this->m_defaultDir;
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
        COSResponse* res = new  COSResponse();
        this->get(url,res);
        if(res->status()){
            response->setStatus(true);
            response->appendLists(res->parseList());
            marker = res->marker();
            response->params = res->params;
            delete res;
            if(marker.isEmpty()){
                break;
            }
        }else{
            response->setStatus(false);
            response->errorMsg = res->errorMsg;
            delete res;
            break;
        }
    }while(true);
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
            //qDebug()<<"remote:"<<remote;
            //QByteArray md5 = QCryptographicHash::hash(task->file->readAll(), QCryptographicHash::Md5).toBase64();
            //qDebug()<<"md5:"<<md5;
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
            //qDebug()<<"header:"<<lists;
            QString url = "http://"+headers[HOST]+this->escape(remote);
            COSResponse* response = new  COSResponse();
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
            //qDebug()<<"headers:"<<m_requestHeaders;
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
            //response->debug();

            if(task->abort==true){
                response->errorCode = -3;
                response->errorMsg = QObject::tr("User aborted");
                task->abort = false;
            }


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

            QString url = "http://"+headers[HOST]+this->escape(remote);
            qDebug()<<"url:"<<url;
            COSResponse* response = new  COSResponse();


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
            //response->debug();
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




NetworkResponse* COS::del(QString bucket,QString dst)
{
    QMutexLocker locker(&mutex);
    HttpParams headers;
    HttpParams params;
    if(dst.startsWith("/")){
        dst = dst.mid(1);
    }
    QStringList arr = this->host.split(".");
    headers[HOST] = this->host;
    QString Authorization = this->signHeaders("DELETE","/"+dst,headers,params);
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

    COSResponse* response = new  COSResponse();

    //curl_easy_setopt(this->curl, CURLOPT_HEADERDATA, (void *)&response->header);
    //curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, (void *)&response->body);
    curl_easy_setopt(this->curl, CURLOPT_HTTPHEADER, chunk);
    QString url = "http://"+headers[HOST]+"/"+this->escape(dst);
    curl_easy_setopt(this->curl,CURLOPT_URL,url.toStdString().c_str());
    curl_easy_setopt(this->curl,CURLOPT_CUSTOMREQUEST,"DELETE");

    this->access(response,false);
    if(size>0){
        m_requestHeaders.clear();
        curl_slist_free_all(chunk);
    }

    /*CURLcode res = curl_easy_perform(this->curl);
    curl_easy_setopt(this->curl,CURLOPT_CUSTOMREQUEST, nullptr);
    curl_slist_free_all(chunk);*/

    return response;
}



NetworkResponse* COS::chDir(const QString &dir)
{
    Q_UNUSED(dir);
    return nullptr;
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
    COSResponse* response = new  COSResponse();
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


NetworkResponse* COS::rename(QString src,QString dst)
{
    QMutexLocker locker(&mutex);
    return nullptr;
}

NetworkResponse* COS::customeAccess(QString name,QMap<QString,QVariant> data)
{
    //return this->errorCode;
    return nullptr;
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
