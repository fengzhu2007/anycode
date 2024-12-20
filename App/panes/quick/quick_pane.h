#ifndef QUICK_PANE_H
#define QUICK_PANE_H
#include <docking_pane.h>

namespace ady{
class QuickPanePrivate;
class QuickPane : public DockingPane
{
public:
    explicit QuickPane(const QString qml,QWidget* parent);
    virtual ~QuickPane();
    virtual QString id() override;
    virtual QString group() override;
    virtual QString description() override;
    virtual void activation() override;
    virtual void save(bool rename) override;
    virtual void contextMenu(const QPoint& pos) override;
    virtual QJsonObject toJson() override;
    virtual bool closeEnable() override;
    virtual void doAction(int a) override;

    static QuickPane* open(DockingPaneManager* dockingManager,bool active=false);
    static QuickPane* make(DockingPaneManager* dockingManager,const QJsonObject& data);
public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;


private:
    QuickPanePrivate* d;
    static QuickPane* instance;

};
}
#endif // QUICK_PANE_H
