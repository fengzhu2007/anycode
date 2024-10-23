#include "network_request.h"
#include "network_response.h"
#include <QString>
#include "transfer/Task.h"
#include "transfer/TaskPoolModel.h"
#include <QDebug>
namespace ady {
    NetworkRequest::NetworkRequest(CURL *curl,long long id)
    {
        this->curl = curl;
        this->id = id;
        this->connected = true;
    }


    NetworkRequest::~NetworkRequest()
    {
        curl_easy_cleanup(this->curl);
    }


    void NetworkRequest::setUsername(const QString& username){this->username = username;}
    void NetworkRequest::setPassword(const QString& password){this->password = password;}
    void NetworkRequest::setPort(int port){this->port = port;}
    void NetworkRequest::setHost(const QString& host){this->host = host;}

    bool NetworkRequest::isConnected()
    {
        return this->connected;
    }


    int NetworkRequest::getErrorCode(){return this->errorCode;}
    QString NetworkRequest::getErrorMsg(){return this->errorMsg;}

    QString NetworkRequest::escape(const QString& str)
    {
        std::string sIn =str.toStdString();
        std::string sOut;
        for( size_t ix = 0; ix < sIn.size(); ix++ )
            {
                unsigned char buf[4];
                memset( buf, 0, 4 );
                if( isspace( (unsigned char)sIn[ix] )){
                    buf[0] = '%';
                    buf[1] = '2';
                    buf[2] = '0';
                }else{
                    buf[0] = sIn[ix];
                }
                sOut += (char *)buf;
            }
            return QString::fromUtf8(sOut.c_str());
    }

    void NetworkRequest::setHeaderHolder(QString* header)
    {
        this->setOption(CURLOPT_HEADERDATA,(void*)header);
        this->setOption(CURLOPT_HEADERFUNCTION, network_response_head);
    }

    void NetworkRequest::setBodyHolder(QString* body)
    {
        this->setOption(CURLOPT_WRITEDATA,(void*)body);
        this->setOption(CURLOPT_WRITEFUNCTION, network_response_body);
    }

    void NetworkRequest::init(const SiteRecord& info){
        Q_UNUSED(info);
    }

    int NetworkRequest::access(NetworkResponse* response,bool body)
    {
        this->setHeaderHolder(&response->header);
        if(body){
            this->setBodyHolder(&response->body);
        }
        //qDebug()<<"body:"<<body;
        //qDebug()<<"body:"<<response->body;
        CURLcode res = curl_easy_perform(this->curl);
        this->errorCode = (int)res;
        if(res!=CURLE_OK){
            this->errorMsg = curl_easy_strerror(res);
        }else{
            this->errorMsg = "";
        }
        response->errorCode = this->errorCode;
        response->errorMsg = this->errorMsg;
        return (int)res;
    }


    NetworkResponse* NetworkRequest::link()
    {
        return nullptr;
    }

    NetworkResponse* NetworkRequest::linkTest()
    {
        return nullptr;
    }

    NetworkResponse* NetworkRequest::listDir(const QString &dir,int page,int pageSize)
    {
        Q_UNUSED(dir);
        Q_UNUSED(page);
        Q_UNUSED(pageSize);
        return nullptr;
    }

    NetworkResponse* NetworkRequest::tinyListDir(const QString& dir)
    {
        Q_UNUSED(dir);
        return nullptr;
    }

    NetworkResponse* NetworkRequest::unlink()
    {
        QMutexLocker locker(&mutex);
        return nullptr;
    }

    NetworkResponse* NetworkRequest::upload(Task* task)
    {
        Q_UNUSED(task);
        return nullptr;
    }

    NetworkResponse* NetworkRequest::download(Task* task)
    {
        Q_UNUSED(task);
        return nullptr;
    }

    NetworkResponse* NetworkRequest::del(Task* task)
    {
        Q_UNUSED(task);
        return nullptr;
    }

    NetworkResponse* NetworkRequest::chmod(Task* task)
    {
        Q_UNUSED(task);
        return nullptr;
    }

    NetworkResponse* NetworkRequest::chDir(const QString &dir){
        return nullptr;
    }


    NetworkResponse* NetworkRequest::mkDir(const QString &dir){
        return nullptr;
    }


    NetworkResponse* NetworkRequest::rmDir(const QString &dir){
        return nullptr;
    }

    NetworkResponse* NetworkRequest::rename(const QString& src,const QString& dst){
        return nullptr;
    }

    NetworkResponse* NetworkRequest::chmod(const QString& dst,int mode){
        return nullptr;
    }

    NetworkResponse* NetworkRequest::del(const QString& dst){
        return nullptr;
    }

    QString NetworkRequest::matchToPath(const QString& from,bool local){
        Q_UNUSED(from);
        Q_UNUSED(local);
        return {};
    }
}



size_t network_response_body(void *ptr, size_t size, size_t nmemb, void *stream)
{
    if (stream == NULL || ptr == NULL || size == 0)
        return 0;
    size_t realsize = size * nmemb;
    QString *buffer = (QString*)stream;
    if (buffer != NULL)
    {
        buffer->append(QString::fromUtf8((const char *)ptr,realsize));
    }
    return realsize;
}

size_t network_response_head(void *ptr, size_t size, size_t nmemb, void *stream)
{
    if (stream == NULL || ptr == NULL || size == 0)
        return 0;
    size_t realsize = size * nmemb;
    QString *buffer = (QString*)stream;
    if (buffer != NULL)
    {
        buffer->append(QString::fromUtf8((const char *)ptr,realsize));
    }
    return realsize;
}

size_t network_read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
   curl_off_t nread;
   ady::Task* task = static_cast<ady::Task*>(stream);
   //qDebug()<<"network_read_callback:"<<task->abort;
   //qDebug()<<"network_read_callback:"<<size<<";"<<nmemb;
   if(task->abort==true){
       return 0;
   }
   nread = task->file->read((char *)ptr,size * nmemb);
   //size_t retcode = fread(ptr, size, nmemb, task->file);
   //nread = (curl_off_t)retcode;
   task->readysize += nread;
   ady::TaskPoolModel::getInstance()->progressTask(task);
   return nread;
}

size_t network_write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    QFile* file = static_cast<QFile*>(stream);
    //qDebug()<<"write size:"<<size * nmemb;
    return file->write((char *)ptr,size * nmemb);

    /*size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
    return written;*/
}

int network_progress_callback(void *p,curl_off_t dltotal, curl_off_t dlnow,curl_off_t ultotal, curl_off_t ulnow)
{
    ady::Task* task = static_cast<ady::Task*>(p);
    if(task->abort==true){
        return 1;
    }
    task->filesize = dltotal;
    task->readysize = dlnow;
    ady::TaskPoolModel::getInstance()->progressTask(task);
    return 0;
}
