#include "NetworkListen.h"
#include <QDebug>
#include "NetworkResponse.h"
//#include "transfer/TaskPoolModel.h"
namespace ady{

NetworkListen::NetworkListen(QObject* parent)
    :QObject(parent),
    NetworkRequest(curl_easy_init())
{
    m_isOnline = false;
    m_second = 5;
    connect(&m_timer,&QTimer::timeout,this,&NetworkListen::onTimeout);
    m_timer.setSingleShot(true);
    m_timer.start(m_second*1000);
}


void NetworkListen::onTimeout(){
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
    m_timer.start(m_second*1000);
    /*qDebug()<<"res:"<<res;
    char *ipstr=NULL;
    res = curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &ipstr);
    qDebug()<<"res2:"<<res;
    qDebug()<<"ip:"<<ipstr;*/
}


NetworkResponse* NetworkListen::customeAccess(QString name,QMap<QString,QVariant> data)
{
    return nullptr;
}

}
