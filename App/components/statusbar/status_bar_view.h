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

        ~StatusBarView();
        bool networkStatus();
        void setNetworkStatus(bool isOnline);
        void showMessage(const QString& message);
        QString currentMessage();
        virtual bool onReceive(Event* e) override;//event bus receive callback
        void setReady();
        void setRate(const QPair<long long,long long>& rate);

        static StatusBarView* getInstance();
        static StatusBarView* make(QWidget* parent);

    private:
        StatusBarView(QWidget* parent=nullptr);

    private:
        static StatusBarView* instance;
        Ui::StatusBarView* ui;
        bool m_isOnline;

    };


}


#endif // STATUSBARVIEW_H
