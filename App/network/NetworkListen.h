#ifndef NETWORKLISTEN_H
#define NETWORKLISTEN_H
#include "NetworkRequest.h"
#include <QObject>
#include <QTimer>
namespace ady{
    class NetworkListen : public QObject, public NetworkRequest
    {
        Q_OBJECT
    public:
        explicit NetworkListen(QObject* parent = nullptr);
        virtual NetworkResponse* customeAccess(QString name,QMap<QString,QVariant> data);

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
#endif // NETWORKLISTEN_H
