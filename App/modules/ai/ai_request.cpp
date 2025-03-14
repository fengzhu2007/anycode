#include "ai_request.h"
#include "network/http/http_response.h"
#include "ai_response.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QtConcurrent>
#include <QThread>
#include <QDebug>
static char prefixKey[] = "prefix";
static char suffixKey[] = "suffix";
static char middleKey[] = "middle";
static char pathKey[] = "path";


namespace ady{
class Worker : public QObject
{
    Q_OBJECT
public:
    Worker(AiRequest* req,const QString& url,const QJsonObject& data):QObject(nullptr),m_req(req),m_url(url),m_data(data){

    }

public slots:
    void doWork() {
        auto response = new AIResponse();
        m_req->post(m_url,m_data,response);
        emit resultReady(response);
    }
signals:
    void resultReady(AIResponse* response);

private:
    AiRequest* m_req;
    QJsonObject m_data;
    QString m_url;
};

AiRequest::AiRequest(QObject* parent,const QString& apiKey,const QJsonObject& data):
    QObject(parent),
    HttpClient(),m_apiKey(apiKey),m_data(data) {

    const QString url = "https://dashscope.aliyuncs.com/compatible-mode/v1/completions";
    //const QString apiKey = "";
    this->addHeader(QString::fromUtf8("Authorization: Bearer %1").arg(m_apiKey));
    this->addHeader(QString::fromUtf8("Content-Type: application/json"));
    this->setOption(CURLOPT_SSL_VERIFYPEER,0l);
    this->setOption(CURLOPT_SSL_VERIFYHOST,0l);
    this->setOption(CURLOPT_TIMEOUT,5);//5m


    QJsonObject json = {
        {"model",this->model()},
        {"prompt",this->prompt()}
    };

    Worker *worker = new Worker(this,url,json);
    worker->moveToThread(&workerThread);

    connect(&workerThread, &QThread::started, worker, &Worker::doWork);
    connect(&workerThread, &QThread::finished, &workerThread, &QThread::deleteLater);
    connect(worker, &Worker::resultReady, this, &AiRequest::handleResults);

}

AiRequest::~AiRequest(){
    qDebug("~AiRequest");
    workerThread.quit();
    workerThread.wait();
}


void AiRequest::call(){
    workerThread.start();
}

void AiRequest::handleResults(AIResponse* response){
    auto sender = static_cast<Worker*>(this->sender());
    delete sender;
    if(this->m_func){
        this->m_func(response);
    }
    emit this->finish();
}

QString AiRequest::model(){
    return QString("qwen2.5-coder-32b-instruct");
}

QString AiRequest::prompt(){
    return QString::fromUtf8("<|fim_prefix|>%1<|fim_suffix|>%2<|fim_middle|>%3").arg(m_data.find(prefixKey)->toString()).arg(m_data.find(suffixKey)->toString()).arg(m_data.find(middleKey)->toString());
}


NetworkResponse* AiRequest::customeAccess(const QString& name,QMap<QString,QVariant> data){
    return nullptr;
}




}
#include "ai_request.moc"
