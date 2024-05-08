#ifndef NETWORKREQUEST_H
#define NETWORKREQUEST_H
#include "global.h"
#include <curl/curl.h>
#include <QString>
#include <QVariant>
#include <QMap>
#include <QMutex>
namespace ady {
    class Task;
    class NetworkResponse;
    class ANYENGINE_EXPORT NetworkRequest
    {
    protected:
        NetworkRequest(CURL* curl);
    public:
        virtual ~NetworkRequest();

        template <typename... P>
        void setOption(CURLoption option,P... args)
        {
            curl_easy_setopt(this->curl,option,args...);
        }

        QString escape(const QString& str);

        void setHeaderHolder(QString* header);
        void setBodyHolder(QString* body);

        void setUsername(QString username);
        void setPassword(QString password);
        void setPort(int port);
        void setHost(QString host);

        bool isConnected();


        int getErrorCode();
        QString getErrorMsg();



        virtual int access(NetworkResponse* response,bool body=true);
        virtual NetworkResponse* link();
        virtual NetworkResponse* linkTest();
        virtual NetworkResponse* listDir(QString dir,int page=1,int pageSize=1000);
        virtual NetworkResponse* tinyListDir(QString dir);
        virtual NetworkResponse* unlink();

        virtual NetworkResponse* customeAccess(QString name,QMap<QString,QVariant> data)=0;

        virtual NetworkResponse* upload(Task* task);
        virtual NetworkResponse* download(Task* task);
        virtual NetworkResponse* del(Task* task);
        virtual NetworkResponse* chmod(Task* task);



    protected:
        CURL* curl;
        int errorCode;
        QString errorMsg;

        QString host;
        QString username;
        QString password;
        int port;

        QString header;
        QString body;

        bool connected;

        QMutex mutex;

        friend class NetworkManager;

    };
}

#ifdef  __cplusplus
extern "C" {
#endif

ANYENGINE_EXPORT size_t network_response_body(void *ptr, size_t size, size_t nmemb, void *stream);
ANYENGINE_EXPORT size_t network_response_head(void *ptr, size_t size, size_t nmemb, void *stream);
ANYENGINE_EXPORT size_t network_read_callback(void *ptr, size_t size, size_t nmemb, void *stream);
ANYENGINE_EXPORT size_t network_write_callback(void *ptr, size_t size, size_t nmemb, void *stream);
ANYENGINE_EXPORT int network_progress_callback(void *p,curl_off_t dltotal, curl_off_t dlnow,curl_off_t ultotal, curl_off_t ulnow);

#ifdef __cplusplus
}
#endif

#endif // NETWORKREQUEST_H
