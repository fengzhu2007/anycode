#ifndef NETWORK_LISTEN_H
#define NETWORK_LISTEN_H
#include "network_request.h"
#include <QObject>
#include <QTimer>
namespace ady{
    class NetworkListen : public QObject, public NetworkRequest
    {
        Q_OBJECT
    public:
        explicit NetworkListen(QObject* parent = nullptr);
        virtual NetworkResponse* customeAccess(const QString& name,QMap<QString,QVariant> data);

    signals:
        void onlineStateChanged(bool isOnline);

    public slots:
        void onTimeout();


    private:
        QTimer m_timer;
        int m_second;
        bool m_isOnline;

    };

}
#endif // NETWORK_LISTEN_H
