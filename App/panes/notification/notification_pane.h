#ifndef NOTIFICATION_PANE_H
#define NOTIFICATION_PANE_H

#include <QWidget>

#include <docking_pane.h>
#include "core/event_bus/subscriber.h"
#include "core/event_bus/event_data.h"


namespace Ui {
class NotificationPane;
}

namespace ady{
class NotificationPanePrivate;
class NotificationPane : public DockingPane , public  Subscriber
{
    Q_OBJECT
public:
    ~NotificationPane();
    static NotificationPane* getInstance();
    void initView();
    virtual QString id() override;
    virtual QString group() override;
    virtual bool onReceive(Event* e) override;//event bus receive callback

    static NotificationPane* open(DockingPaneManager* dockingManager,bool active=false);
    static NotificationPane* make(DockingPaneManager* dockingManager,const QJsonObject& data);

    static void notify(const NotificationData& data);
public slots:
    void onActionTriggered();

private:
    explicit NotificationPane(QWidget *parent = nullptr);

public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;

private:
    Ui::NotificationPane *ui;
    NotificationPanePrivate* d;
    static NotificationPane* instance;
    static QList<NotificationData> list;
};
}
#endif // NOTIFICATION_PANE_H
