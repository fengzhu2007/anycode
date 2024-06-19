#ifndef RESOURCE_MANAGER_PANE_H
#define RESOURCE_MANAGER_PANE_H

#include "docking_pane.h"
#include "core/event_bus/subscriber.h"

namespace Ui {
class ResourceManagerPane;
}

namespace ady{
class ResourceManagerPanePrivate;
class ResourceManagerPane : public DockingPane , public Subscriber
{
    Q_OBJECT

public:
    explicit ResourceManagerPane(QWidget *parent = nullptr);
    ~ResourceManagerPane();
    virtual QString id() override;
    virtual QString group() override;
    virtual bool onReceive(Event* e) override;//event bus receive callback

private:
    Ui::ResourceManagerPane *ui;
    ResourceManagerPanePrivate* d;
    static const QString PANE_ID;
    static const QString PANE_GROUP;


    //event message list id
    static const QString M_OPEN_PROJECT;
};

}
#endif // RESOURCE_MANAGER_PANE_H
