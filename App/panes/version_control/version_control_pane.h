#ifndef VERSIONCONTROLPANE_H
#define VERSIONCONTROLPANE_H
#include "global.h"
#include "docking_pane.h"
#include "w_spin.h"
#include "core/event_bus/subscriber.h"
#include <QItemSelection>

namespace Ui {
class VersionControlPane;
}
namespace ady{
namespace cvs{
class Repository;
}
class VersionControlPanePrivate;
class ANYENGINE_EXPORT VersionControlPane : public DockingPane ,public wSpin , public Subscriber
{
    Q_OBJECT
public:
    ~VersionControlPane();
    void initView();
    virtual QString id() override;
    virtual QString group() override;
    virtual bool onReceive(Event* e) override;//event bus receive callback
    void queryCommit(bool refresh=false);
    void clearCommit();
    void queryDiff(const QString& oid1,const QString& oid2);
    void clearDiff();
    void clearProject();

    static VersionControlPane* open(DockingPaneManager* dockingManager,bool active=false);
    static VersionControlPane* getInstance(){return instance;}

    static VersionControlPane* make(DockingPaneManager* dockingManager,const QJsonObject& data);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

public slots:
    void onProjectChanged(int i);
    void onUpdateCommit(int rid,void* list);
    void onUpdateDiff(const QString& first,const QString& second,int rid,void* list);
    void onQueryCommit(int pos);
    void onCommitSelected(const QModelIndex &current, const QModelIndex &previous);
    void onCommitSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onActionTriggered();
    void onError(int rid,int code,const QString& message);
    void onCommitContextMenu(const QPoint& pos);
    void onDiffContextMenu(const QPoint& pos);
    void onMarkAs(bool checked=false);

private:
    VersionControlPane(QWidget *parent = nullptr);

public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;

private:
    VersionControlPanePrivate* d;
    Ui::VersionControlPane *ui;
    static VersionControlPane* instance;
};
}

#endif // VERSIONCONTROLPANE_H
