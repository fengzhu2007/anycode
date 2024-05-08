#ifndef NOTIFICATION_H
#define NOTIFICATION_H
#include <QWidget>
#include "global.h"
#include <QTimer>
namespace Ui {
    class NotificationItemUI;
}
namespace ady {
    class ANYENGINE_EXPORT NotificationItem : public QWidget
    {
        constexpr const static char INFO_QSS[] = "#frame{border:1px solid #91d5ff;border-radius:5px;background-color:rgba(230, 247, 255,80%)}";
        constexpr const static char SUCCESS_QSS[] = "#frame{border:1px solid #b7eb8f;border-radius:5px;background-color:rgba(246, 255, 237,80%)}";
        constexpr const static char WARNING_QSS[] = "#frame{border:1px solid #ffe58f;border-radius:5px;background-color:rgba(255, 251, 230,80%)}";
        constexpr const static char ERROR_QSS[] = "#frame{border:1px solid #ffa39e;border-radius:5px;background-color:rgba(255, 241, 240,80%)}";


        Q_OBJECT
    public:
        enum Status {
            Info=0,//blue
            Success,//green
            Warning,//yellow
            Error//red
        };

        NotificationItem(QWidget* parent);

        void setStatus(Status status);
        void setLabel(QString label);
        void setCloseable(bool closeable);
        void setShowTime(qint64 ms);
        inline long long dataId(){return m_id;}

    public slots:
        void onClose();

    /*signals:
        void close(NotificationItem*);*/



    public:
         static long long seq ;

    private:
        long long m_id;
        Status m_status;
        QString m_label;
        bool m_closeable;
        qint64 m_showtime;
        QTimer m_timer;
        Ui::NotificationItemUI* ui;

    };


    class ANYENGINE_EXPORT Notification : public QWidget
    {
        Q_OBJECT
    public :
        Notification(QWidget* parent);

        NotificationItem* notify(QString text,NotificationItem::Status status=NotificationItem::Info,qint64 showtime=0,long long nid=0l);
        void resize();

    private:
        unsigned int m_maxCount;
        int m_space = 4;
    };



}
#endif // NOTIFICATION_H
