#ifndef AI_REQUEST_H
#define AI_REQUEST_H
#include "global.h"
#include "network/http/http_client.h"
#include <QJsonObject>
#include <QThread>
#include <functional>

namespace ady{
class AIResponse;

using AIResponseCallbackFunc = std::function<void(AIResponse*)>;


class ANYENGINE_EXPORT AiRequest :public QObject, public HttpClient
{
    Q_OBJECT
public:
    AiRequest(QObject* parent,const QString& apiKey,const QJsonObject& data);
    ~AiRequest();

    virtual void call();
    virtual QString model();
    virtual QString prompt();

    virtual NetworkResponse* customeAccess(const QString& name,QMap<QString,QVariant> data) override;

    void setCallbackResponse(AIResponseCallbackFunc func){
        m_func = func;
    }


signals:
    void finish();
    void operate(AiRequest* req,const QString& url,const QJsonObject& data);

public slots:
    void handleResults(AIResponse* response);

private:
    QThread workerThread;
    QJsonObject m_data;
    QString m_apiKey;
    AIResponseCallbackFunc m_func;


};
}
#endif // AI_REQUEST_H
