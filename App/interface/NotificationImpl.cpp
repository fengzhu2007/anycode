#include "NotificationImpl.h"
#include "components/Notification.h"
namespace ady {
    NotificationImpl::NotificationImpl(QWidget* parent)
    {
        m_notification = new Notification(parent);
        m_notification->hide();
    }

    Notification* NotificationImpl::notification()
    {
        return m_notification;
    }

    NotificationImpl::~NotificationImpl()
    {
        delete m_notification;
    }

    long long NotificationImpl::notify(QString text,int status,qint64 ms,long long nid)
    {
        return m_notification->notify(text,(NotificationItem::Status)status,ms,nid)->dataId();
    }
}
