#include "ssl_querier.h"
#include "ssl_query_task.h"

#include <QJsonObject>
#include <QDebug>

namespace ady{

SSLQuerier::SSLQuerier(const QStringList& list,QObject* parent)
    :QObject(parent),
    NetworkRequest(curl_easy_init()),m_sitelist(list)
{
}

SSLQuerier::~SSLQuerier(){
    qDebug()<<"~SSLQuerier";
}


bool SSLQuerier::execute(){
    this->setOption(CURLOPT_TIMEOUT,3l);
    this->setOption(CURLOPT_NOBODY,1);
    this->setOption(CURLOPT_HEADER,0);
    this->setOption(CURLOPT_SSL_VERIFYPEER, 0L);
    this->setOption(CURLOPT_SSL_VERIFYHOST, 0L);
    this->setOption(CURLOPT_CERTINFO,1l);
    for(auto one:m_sitelist){
        QString url = "https://"+one;
        this->setOption(CURLOPT_URL,url.toStdString().c_str());
        CURLcode res = curl_easy_perform(this->curl);
        //qDebug()<<"url"<<url<<res;
        if(!res){
            this->parseCertInfo(one);
        }else{
            emit oneError(one,"Request error");
        }
    }
    emit finish();
    this->deleteLater();
    return true;
}

void SSLQuerier::parseCertInfo(const QString& domain){
    struct curl_certinfo *ci;
    CURLcode res = curl_easy_getinfo(curl, CURLINFO_CERTINFO, &ci);
    if (res) {
        emit oneError(domain,"error");
    }else{
        for (int i = 0; i < ci->num_of_certs; i++) {
            QJsonObject json;
            struct curl_slist *slist;
            for (slist = ci->certinfo[i]; slist; slist = slist->next){
                QString data = QString::fromUtf8(slist->data);
                if(data.startsWith("Subject:")){
                    json.insert("Subject",data.mid(8));
                }else if(data.startsWith("Start date:")){
                    json.insert("StartDate",data.mid(11));
                }else if(data.startsWith("Expire date:")){
                    json.insert("ExpireDate",data.mid(12));
                }
            }
            emit oneReady(domain,json);
            break;
        }
    }
}



NetworkResponse* SSLQuerier::customeAccess(const QString& name,QMap<QString,QVariant> data){
    return nullptr;
}

}
