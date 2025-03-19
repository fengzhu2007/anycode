#ifndef TERMINAL_PANE_H
#define TERMINAL_PANE_H

#include "global.h"
#include "docking_pane.h"
#include "core/event_bus/subscriber.h"
#include <QIcon>

namespace Ui {
class TerminalPane;
}

namespace ady{


class TerminalPanePrivate;
class ANYENGINE_EXPORT TerminalPane : public DockingPane , public Subscriber
{
public:
    enum TerminalType{
        Unkown=0,
        Cmd,
        PowerShell,
        Shell
    };
    struct Excutable{
        TerminalPane::TerminalType type;
        QIcon icon;
        QString name;
        QString executable;
    };

    ~TerminalPane();

    void initView();
    virtual QString id() override;
    virtual QString group() override;
    virtual bool onReceive(Event* e) override;//event bus receive callback
    virtual QJsonObject toJson() override;

    void newTermnal(TerminalPane::TerminalType type,const QString& workingDir);
    void newTermnal(const QString& excutablePath,const QString& workingDir);


    static TerminalPane* getInstance();
    static TerminalPane* open(DockingPaneManager* dockingManager,bool active=false,const QJsonObject& data={});
    static TerminalPane* make(DockingPaneManager* dockingManager,const QJsonObject& data);
    static QList<TerminalPane::Excutable> toExecutableList();


public slots:
    void onActionTriggered();
    void onCurrentChanged(int index);

private:
    explicit TerminalPane(QWidget *parent = nullptr,const QString& executable={},const QString& workingDir={});
    void updateToolBar();


public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;

private:
    Ui::TerminalPane *ui;
    TerminalPanePrivate* d;
    static TerminalPane* instance;
    //static QMap<Type,QString> executablelist;
    static QList<TerminalPane::Excutable> executablelist;
};

}
#endif // TERMINAL_PANE_H
