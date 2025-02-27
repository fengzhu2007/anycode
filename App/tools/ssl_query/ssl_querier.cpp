#include "ssl_querier.h"
#include "ssl_query_task.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event_data.h"
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

namespace ady{

SSLQuerier::SSLQuerier(const QStringList& list,QObject* parent)
    :QObject(parent),
    NetworkRequest(curl_easy_init()),m_sitelist(list)
{
    m_errorCount = 0;

    connect(this,&SSLQuerier::notify,this,&SSLQuerier::onNotify);
}

SSLQuerier::~SSLQuerier(){
    //qDebug()<<"~SSLQuerier";
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
            m_errorCount += 1;
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
        m_errorCount += 1;
        emit oneError(domain,"error");
    }else{
        QDateTime current = QDateTime::currentDateTime();
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
                    QDateTime datetime = parseDateTime(data.mid(12));
                    auto ExpireDate = data.mid(12);
                    json.insert("ExpireDate",data.mid(12));
                    int ret = current.secsTo(datetime);
                    if(ret < 5*86400){
                        //less than 5 days
                        m_errorCount += 1;

                        emit notify(domain,ExpireDate);
                    }
                }
            }
            emit oneReady(domain,json);
            break;
        }
    }
}

QDateTime SSLQuerier::parseDateTime(const QString& dateTimeString){
    QStringList parts = dateTimeString.simplified().split(' ');
    if (parts.size() == 5) {
        QString monthStr = parts[0]; // "Jul"
        int day = parts[1].toInt();  // 8
        QString timeStr = parts[2];  // "01:41:02"
        int year = parts[3].toInt(); // 2024
        QString timeZone = parts[4]; // "GMT"
        QMap<QString, int> monthMap = {
            {"Jan", 1}, {"Feb", 2}, {"Mar", 3}, {"Apr", 4},
            {"May", 5}, {"Jun", 6}, {"Jul", 7}, {"Aug", 8},
            {"Sep", 9}, {"Oct", 10}, {"Nov", 11}, {"Dec", 12}
        };
        int month = monthMap.value(monthStr, -1);
        if (month != -1) {
            QStringList timeParts = timeStr.split(':');
            if (timeParts.size() == 3) {
                int hour = timeParts[0].toInt();
                int minute = timeParts[1].toInt();
                int second = timeParts[2].toInt();
                QDate date(year, month, day);
                QTime time(hour, minute, second);
                QDateTime dateTime(date, time,Qt::UTC);
                return dateTime;
            }
        }
    }
    return {};
}

void SSLQuerier::onNotify(const QString& domain,const QString& expireDate){
    QDateTime current = QDateTime::currentDateTime();
    NotificationData data{"SSL",tr("Certificate expiration reminder"),tr("The expiration time of the SSL certificate for the %1 domain name is %2, please note!").arg(domain).arg(expireDate),current.toString("MM-dd HH:mm"),{}};
    Publisher::getInstance()->post(Type::M_NOTIFICATION,&data);
}


NetworkResponse* SSLQuerier::customeAccess(const QString& name,QMap<QString,QVariant> data){
    return nullptr;
}

}
