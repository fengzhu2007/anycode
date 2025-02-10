#ifndef SSL_QUERIER_H
#define SSL_QUERIER_H
#include "network/network_request.h"
#include <QObject>

namespace ady{
class NetworkResponse;
class SSLQuerier : public QObject, public NetworkRequest
{
    Q_OBJECT
public:
    explicit SSLQuerier(const QStringList& list,QObject* parent = nullptr);
    virtual NetworkResponse* customeAccess(const QString& name,QMap<QString,QVariant> data) override;
    bool execute();

signals:
    void oneReady();

private:
    QStringList m_sitelist;

};

}

#endif // SSL_QUERIER_H
