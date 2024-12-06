#ifndef NETWORK_LISTEN_H
#define NETWORK_LISTEN_H
#include "network_request.h"
#include <QObject>
#include <QTimer>
namespace ady{
class NetworkResponse;
    class NetworkListen : public QObject, public NetworkRequest
    {
        Q_OBJECT
    public:
        explicit NetworkListen(QObject* parent = nullptr);
        virtual NetworkResponse* customeAccess(const QString& name,QMap<QString,QVariant> data) override;
        bool execute();

    signals:
        void onlineStateChanged(bool isOnline);

    private:
        bool m_isOnline;

    };

}
#endif // NETWORK_LISTEN_H
