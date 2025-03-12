#include "ai_request.h"
#include "network/http/http_response.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
static char prefixKey[] = "prefix";
static char suffixKey[] = "suffix";
static char middleKey[] = "middle";
static char pathKey[] = "path";


namespace ady{
AiRequest::AiRequest(QObject* parent,const QJsonObject& data):
    QObject(parent),
    HttpClient(),m_data(data) {

}

AiRequest::~AiRequest(){
    qDebug("~AiRequest");
}


HttpResponse* AiRequest::call(){
    const QString url = "https://dashscope.aliyuncs.com/compatible-mode/v1/completions";
    const QString apiKey = "";
    this->addHeader(QString::fromUtf8("Authorization: Bearer %1").arg(apiKey));
    this->addHeader(QString::fromUtf8("Content-Type: application/json"));
    this->setOption(CURLOPT_SSL_VERIFYPEER,0l);
    this->setOption(CURLOPT_SSL_VERIFYHOST,0l);
    //qDebug()<<this->prompt();
    QJsonObject data = {
        {"model",this->model()},
        {"prompt",this->prompt()}
    };
    auto response = this->post(url,data);
    if(response->status()){
        //qDebug()<<"body"<<response->body;

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(response->body.toUtf8(),&error);
        if(error.error==QJsonParseError::NoError){
            QJsonObject data = doc.object();
            auto choices = data.find("choices")->toArray();
            if(choices.size()>0){
                auto code = choices.at(0).toObject().find("text")->toString();
                qDebug()<<"code"<<code;
            }

        }
    }else{
        response->debug();
    }
    return response;
}

QString AiRequest::model(){
    //return QString("qwen2.5-coder-32b-instruct");
    return QString("deepseek-v3");
}

QString AiRequest::prompt(){
    return QString::fromUtf8("<|fim_prefix|>%1<|fim_suffix|>%2<|fim_middle|>%3").arg(m_data.find(prefixKey)->toString()).arg(m_data.find(suffixKey)->toString()).arg(m_data.find(middleKey)->toString());

}


NetworkResponse* AiRequest::customeAccess(const QString& name,QMap<QString,QVariant> data){
    return nullptr;
}




}
