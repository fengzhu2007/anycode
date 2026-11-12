#include "network_request.h"
#include "network_response.h"
#include <QString>
#include <QByteArray>
#include <QDateTime>
#include "transfer/task.h"
//#include "transfer/TaskPoolModel.h"
#include "panes/file_transfer/file_transfer_model.h"
#include <QDebug>
namespace ady {
    NetworkRequest::NetworkRequest(CURL *curl,long long id)
    {
        this->curl = curl;
        this->id = id;
        this->connected = true;
        this->m_last_request_time = 0;
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
        //ftp url path 转义:保留 unreserved 字符与 '/'，其余按 UTF-8 字节做百分号编码
        //旧实现只处理空格，文件名中含 # ? % 等字符时会截断或请求错误路径
        static const char hex[] = "0123456789ABCDEF";
        const QByteArray in = str.toUtf8();
        QByteArray out;
        out.reserve(in.size() + 8);
        const int size = in.size();
        for(int i=0;i<size;i++){
            const unsigned char ch = (unsigned char)in.at(i);
            if((ch>='a'&&ch<='z') || (ch>='A'&&ch<='Z') || (ch>='0'&&ch<='9')
                || ch=='-' || ch=='_' || ch=='.' || ch=='~' || ch=='/'){
                out.append((char)ch);
            }else{
                out.append('%');
                out.append(hex[ch>>4]);
                out.append(hex[ch&0x0f]);
            }
        }
        return QString::fromLatin1(out);
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

    QString NetworkRequest::matchToPath(const QString& from,bool is_file,bool local){
        Q_UNUSED(from);
        Q_UNUSED(is_file);
        Q_UNUSED(local);
        return from;
    }

    void NetworkRequest::autoClose(long long current){
        Q_UNUSED(current);
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

namespace {
    //进度上报节流:回调里只做累加,按时间/进度步长限量触发跨线程事件
    //否则大文件传输时每个数据块(16~64KB)都会产生一次 UI 事件,10GB 文件约 16 万次
    constexpr long long PROGRESS_REPORT_INTERVAL = 200;//ms,最长上报间隔
    constexpr long long PROGRESS_MIN_INTERVAL = 50;//ms,两次上报的最小间隔
    constexpr float PROGRESS_REPORT_STEP = 0.005f;//0.5%,进度步长

    inline bool shouldReportProgress(ady::Task* task,float progress)
    {
        const long long now = QDateTime::currentMSecsSinceEpoch();
        if(progress>=1.0f){
            //完成状态只报一次
            if(task->notify_progress>=1.0f){
                return false;
            }
            task->notify_progress = 1.0f;
            task->notify_time = now;
            return true;
        }
        const long long span = now - task->notify_time;
        if(span>=PROGRESS_REPORT_INTERVAL
            || (span>=PROGRESS_MIN_INTERVAL && progress - task->notify_progress >= PROGRESS_REPORT_STEP)){
            task->notify_progress = progress;
            task->notify_time = now;
            return true;
        }
        return false;
    }

    inline void reportProgress(ady::Task* task,long long loaded,float progress,bool upload)
    {
        long long size = loaded - task->notify_size;
        if(size<0){
            size = 0;
        }
        task->notify_size = loaded;
        ady::FileTransferModel::getInstance()->progress(task->siteid,task->id,progress,upload,size);
    }
}

size_t network_read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
   ady::Task* task = static_cast<ady::Task*>(stream);
   if(task==nullptr || task->file==nullptr){
       return 0;
   }
   if(task->abort.load()){
       return 0;
   }
   qint64 nread = task->file->read((char *)ptr,size * nmemb);
   if(nread<=0){
       //读失败交给 curl 报 CURLE_READ_ERROR，不能返回一个假成功值
       return 0;
   }
   task->readysize += nread;
   float progress = task->filesize>0 ? task->readysize*1.0f / task->filesize : 0.0f;
   if(progress>1.0f){
       progress = 1.0f;
   }
   if(shouldReportProgress(task,progress)){
       reportProgress(task,task->readysize,progress,true);
   }
   return (size_t)nread;
}

size_t network_write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    QFile* file = static_cast<QFile*>(stream);
    if(file==nullptr){
        return 0;
    }
    qint64 ret = file->write((char *)ptr,size * nmemb);
    return ret>0 ? (size_t)ret : 0;
}

int network_progress_callback(void *p,curl_off_t dltotal, curl_off_t dlnow,curl_off_t ultotal, curl_off_t ulnow)
{
    (void)ultotal;
    (void)ulnow;
    ady::Task* task = static_cast<ady::Task*>(p);
    if(task==nullptr){
        return 0;
    }
    if(task->abort.load()){
        return 1;
    }
    if(dltotal>0){
        task->filesize = dltotal;
    }
    //未知文件大小时不计算百分比，避免除零与进度跳变
    float progress = task->filesize>0 ? float(dlnow*1.0/task->filesize) : 0.0f;
    if(progress>1.0f){
        progress = 1.0f;
    }else if(progress<0.0f){
        progress = 0.0f;
    }
    if(shouldReportProgress(task,progress)){
        reportProgress(task,dlnow,progress,false);
    }
    return 0;
}
