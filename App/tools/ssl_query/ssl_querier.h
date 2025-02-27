#ifndef SSL_QUERIER_H
#define SSL_QUERIER_H
#include "network/network_request.h"
#include <QObject>
#include <QDateTime>

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

    static QDateTime parseDateTime(const QString& dateTimeString);
private:
    void parseCertInfo(const QString& domain);

signals:
    void oneReady(const QString& domain,const QJsonObject& data);
    void oneError(const QString& domain,const QString& errorMsg);
    void notify(const QString& domain,const QString& expireDate);
    void finish();

public slots:
    void onNotify(const QString& domain,const QString& expireDate);


private:
    QStringList m_sitelist;
    SSLQueryCallback* m_callback;
    int m_errorCount;

};

}

#endif // SSL_QUERIER_H
