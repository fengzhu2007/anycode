#ifndef OUTPUT_PANE_H
#define OUTPUT_PANE_H
#include <docking_pane.h>
#include "core/event_bus/subscriber.h"

namespace Ui {
class OutputPane;
}

namespace ady{
class OutputPanePrivate;
class OutputPane : public DockingPane , public Subscriber
{
    Q_OBJECT

public:
    ~OutputPane();

    void initView();
    virtual QString id() override;
    virtual QString group() override;
    virtual bool onReceive(Event* e) override;//event bus receive callback

    static OutputPane* open(DockingPaneManager* dockingManager,bool active=false);
    static OutputPane* make(DockingPaneManager* dockingManager,const QJsonObject& data);

public slots:
    void onActionTriggered();
    void onIndexChanged(int index);


private:
    explicit OutputPane(QWidget *parent = nullptr);


public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;


private:
    Ui::OutputPane *ui;
    OutputPanePrivate* d;
    static OutputPane* instance;
};
}

#endif // OUTPUT_PANE_H
