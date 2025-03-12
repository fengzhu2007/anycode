#ifndef AI_REQUEST_H
#define AI_REQUEST_H
#include "global.h"
#include "network/http/http_client.h"
#include <QJsonObject>


namespace ady{
class ANYENGINE_EXPORT AiRequest :public QObject, public HttpClient
{
    Q_OBJECT
public:
    AiRequest(QObject* parent,const QJsonObject& data);
    ~AiRequest();

    virtual HttpResponse* call();
    virtual QString model();
    virtual QString prompt();

    virtual NetworkResponse* customeAccess(const QString& name,QMap<QString,QVariant> data) override;


signals:

private:
    QJsonObject m_data;


};
}
#endif // AI_REQUEST_H
