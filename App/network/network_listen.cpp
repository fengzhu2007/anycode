#include "network_listen.h"
#include "network_response.h"
#include <QDebug>
namespace ady{

NetworkListen::NetworkListen(QObject* parent)
    :QObject(parent),
    NetworkRequest(curl_easy_init())
{
    m_isOnline = true;
}


bool NetworkListen::execute(){
    this->setOption(CURLOPT_TIMEOUT,2l);
    this->setOption(CURLOPT_URL,"www.qq.com");
    this->setOption(CURLOPT_NOBODY,1);
    this->setOption(CURLOPT_HEADER,0);
    CURLcode res = curl_easy_perform(this->curl);
    bool isOnline = res == CURLE_OK;
    if(m_isOnline!=isOnline){
        m_isOnline = isOnline;
        emit onlineStateChanged(isOnline);
    }
    return isOnline;
}

NetworkResponse* NetworkListen::customeAccess(const QString& name,QMap<QString,QVariant> data){
    return nullptr;
}

}
