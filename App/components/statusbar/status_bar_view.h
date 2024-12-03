#ifndef STATUSBARVIEW_H
#define STATUSBARVIEW_H
#include <QWidget>
#include "core/event_bus/subscriber.h"
namespace Ui {
    class StatusBarView;
}
namespace ady{
    class StatusBarView : public QWidget , public Subscriber
    {
        Q_OBJECT
    public:
        StatusBarView(QWidget* parent=nullptr);
        ~StatusBarView();
        bool networkStatus();
        void setNetworkStatus(bool isOnline);
        void showMessage(const QString& message);
        QString currentMessage();
        virtual bool onReceive(Event* e) override;//event bus receive callback
        void setReady();

    private:
        Ui::StatusBarView* ui;
        bool m_isOnline;

    };


}


#endif // STATUSBARVIEW_H
