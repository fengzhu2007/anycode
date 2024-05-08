#include "HttpClient.h"
#include "HttpResponse.h"
namespace ady{

HttpClient::HttpClient()
    :NetworkRequest(curl_easy_init())
{

}

HttpClient::HttpClient(CURL* curl)
    :NetworkRequest(curl)
{

}

int HttpClient::access(NetworkResponse* response,bool body)
{
    int ret =NetworkRequest::access(response,body);
    response->parse();
    return ret;
}

HttpResponse* HttpClient::post(QString url,QMap<QString,QString> data,HttpResponse* response)
{
    this->setOption(CURLOPT_URL,url.toUtf8().constData());
    QMap<QString,QString>::iterator iter = data.begin();
    while(iter!=data.end()){
        curl_formadd(&m_postData, &m_postLast, CURLFORM_COPYNAME, iter.key().toUtf8().constData(),
                                  CURLFORM_COPYCONTENTS, iter.value().toUtf8().constData(), CURLFORM_END);
        iter++;
    }
    this->setOption(CURLOPT_POST,1);
    this->setOption(CURLOPT_HTTPPOST,m_postData);

    struct curl_slist *chunk = nullptr;
    for(auto header:m_requestHeaders){
        chunk = curl_slist_append(chunk, header.toUtf8().constData());
    }
    this->setOption(CURLOPT_HTTPHEADER,chunk);
    this->setOption(CURLOPT_URL,url.toUtf8().constData());
    if(response==nullptr){
        response = new HttpResponse;
    }
    this->access(response);

    curl_formfree(m_postData);
    if(chunk!=nullptr){
        curl_slist_free_all(chunk);
    }

    m_postData = nullptr;
    m_postLast = nullptr;
    m_requestHeaders.clear();

    return response;

}

HttpResponse* HttpClient::get(QString url,HttpResponse* response)
{
    this->setOption(CURLOPT_URL,url.toStdString().c_str());
    this->setOption(CURLOPT_POST,0);
    this->setOption(CURLOPT_HTTPPOST,nullptr);
    this->setOption(CURLOPT_CUSTOMREQUEST,"GET");
    int size = m_requestHeaders.size();
    struct curl_slist *chunk = nullptr;
    if(size>0){
        for(auto header:m_requestHeaders){
            chunk = curl_slist_append(chunk, header.toUtf8().constData());
        }
        this->setOption(CURLOPT_HTTPHEADER,chunk);
    }
    if(response==nullptr){
        response = new HttpResponse;
    }
    this->access(response);
    if(size>0){
        m_requestHeaders.clear();
        curl_slist_free_all(chunk);
    }
    return response;
}

void HttpClient::addFile(QString filepath,QString name)
{
    curl_formadd(&m_postData, &m_postLast,
                    CURLFORM_COPYNAME, name.toUtf8().constData(),
                    CURLFORM_FILE,filepath.toLocal8Bit().constData(),
                    CURLFORM_END
            );
}

void HttpClient::addHeader(QStringList headers)
{
    m_requestHeaders<<headers;
}

void HttpClient::addHeader(QString header)
{
    m_requestHeaders.push_back(header);
}

void HttpClient::clearHeader()
{
    m_requestHeaders.clear();
}

QString HttpClient::scheme()
{
    if(this->port==443){
        return "https";
    }else{
        return "http";
    }
}

}
