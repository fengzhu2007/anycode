#ifndef GATEWAY_REQUEST_H
#define GATEWAY_REQUEST_H
#include "global.h"
#include "network/http/http_client.h"
#include <QJsonObject>
#include <QThread>
#include <QString>
#include <functional>

namespace ady{
class GatewayResponse;

using GatewayResponseCallbackFunc = std::function<void(GatewayResponse*)>;

class ANYENGINE_EXPORT GatewayRequest : public QObject, public HttpClient
{
    Q_OBJECT
public:
    GatewayRequest(QObject* parent, const QJsonObject& data);
    ~GatewayRequest();

    virtual void call();
    virtual QString model();
    virtual QString prompt();

    virtual NetworkResponse* customeAccess(const QString& name, QMap<QString,QVariant> data) override;

    void setCallbackResponse(GatewayResponseCallbackFunc func){
        m_func = func;
    }

    volatile bool m_abort = false;  // set true in destructor to cancel SSE curl transfer

signals:
    void finish();
    void chunkReceived(const QString &content);
    void operate(GatewayRequest* req, const QString& url, const QJsonObject& data);

public slots:
    void handleResults(GatewayResponse* response);

private:
    QThread workerThread;
    QJsonObject m_data;
    GatewayResponseCallbackFunc m_func;
};
}
#endif // GATEWAY_REQUEST_H
