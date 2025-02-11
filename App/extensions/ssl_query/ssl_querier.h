#ifndef SSL_QUERIER_H
#define SSL_QUERIER_H
#include "network/network_request.h"
#include <QObject>


namespace ady{
class NetworkResponse;
class SSLQueryCallback;
class SSLQuerier : public QObject, public NetworkRequest
{
    Q_OBJECT
public:
    explicit SSLQuerier(const QStringList& list,QObject* parent = nullptr);
    ~SSLQuerier();
    virtual NetworkResponse* customeAccess(const QString& name,QMap<QString,QVariant> data) override;
    bool execute();

private:
    void parseCertInfo(const QString& domain);

signals:
    void oneReady(const QString& domain,const QJsonObject& data);
    void oneError(const QString& domain,const QString& errorMsg);
    void finish();

private:
    QStringList m_sitelist;
    SSLQueryCallback* m_callback;

};

}

#endif // SSL_QUERIER_H
