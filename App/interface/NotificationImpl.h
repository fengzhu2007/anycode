#ifndef NOTIFICATIONIMPL_H
#define NOTIFICATIONIMPL_H
#include "global.h"
#include <QWidget>
namespace ady {
    class Notification;
    class ANYENGINE_EXPORT NotificationImpl
    {
    public:
        NotificationImpl(QWidget* parent);
        Notification* notification();
        ~NotificationImpl();
        long long notify(QString text,int status=0,qint64 ms=0,long long nid=0);

    protected:
        virtual void resizeEvent(QResizeEvent *event)=0;



    protected:
        Notification* m_notification;

    };
}
#endif // NOTIFICATIONIMPL_H
