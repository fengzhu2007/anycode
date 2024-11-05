#ifndef TERMINAL_PANE_H
#define TERMINAL_PANE_H

#include "global.h"
#include "docking_pane.h"
#include "core/event_bus/subscriber.h"


namespace Ui {
class TerminalPane;
}

namespace ady{
class TerminalPanePrivate;
class ANYENGINE_EXPORT TerminalPane : public DockingPane , public Subscriber
{
public:

    ~TerminalPane();

    void initView();
    virtual QString id() override;
    virtual QString group() override;
    virtual bool onReceive(Event* e) override;//event bus receive callback


    static TerminalPane* open(DockingPaneManager* dockingManager,bool active=false);
    static TerminalPane* make(DockingPaneManager* dockingManager,const QJsonObject& data);

private:
    explicit TerminalPane(QWidget *parent = nullptr);


public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;

private:
    Ui::TerminalPane *ui;
    TerminalPanePrivate* d;
    static TerminalPane* instance;

};

}
#endif // TERMINAL_PANE_H
