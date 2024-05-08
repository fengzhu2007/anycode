#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H
#include "global.h"
#include "network/NetworkRequest.h"
#include <QMap>
#include <QString>
typedef QMap<QString,QString> HttpParams;
typedef QMap<QString,QString>::iterator HttpParamIter;


namespace ady {
    class HttpResponse;
    class ANYENGINE_EXPORT HttpClient : public NetworkRequest
    {
    public:
        HttpClient();
        HttpClient(CURL* curl);

        virtual int access(NetworkResponse* response,bool body=true);

        HttpResponse* post(QString url,QMap<QString,QString> data = QMap<QString,QString>(),HttpResponse* response=nullptr);
        HttpResponse* get(QString url,HttpResponse* response=nullptr);
        void addFile(QString filepath,QString name);
        void addHeader(QStringList headers);
        void addHeader(QString header);
        void clearHeader();

    protected:
        QString scheme();


    protected:
        //QList<QString> m_requestHeaders;
        QStringList m_requestHeaders;
        struct curl_httppost *m_postData = nullptr;
        struct curl_httppost *m_postLast = nullptr;


    };
}
#endif // HTTPCLIENT_H
