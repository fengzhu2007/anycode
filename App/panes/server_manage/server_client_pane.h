#ifndef SERVERCLIENTPANE_H
#define SERVERCLIENTPANE_H
#include "global.h"
#include "docking_pane.h"
#include "core/event_bus/subscriber.h"

namespace ady{
class Panel;
class ServerClientPanePrivate;
class ANYENGINE_EXPORT ServerClientPane : public DockingPane  , public Subscriber
{
    Q_OBJECT

public:
    ServerClientPane(QWidget* parent,long long id);

    ~ServerClientPane();
    void initView();
    void setCenterWidget(Panel* widget);
    virtual QString id() override;
    virtual QString group() override;
    virtual bool onReceive(Event* e) override;//event bus receive callback




public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;

private:
    ServerClientPanePrivate* d;


};
}
#endif // SERVERCLIENTPANE_H
