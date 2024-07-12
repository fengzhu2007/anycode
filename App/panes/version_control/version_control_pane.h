#ifndef VERSIONCONTROLPANE_H
#define VERSIONCONTROLPANE_H
#include "global.h"
#include "docking_pane.h"
#include "core/event_bus/subscriber.h"
#include "cvs/commit.h"

namespace Ui {
class VersionControlPane;
}
namespace ady{
namespace cvs{
class Repository;
}
class VersionControlPanePrivate;
class ANYENGINE_EXPORT VersionControlPane : public DockingPane , public Subscriber
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

    static VersionControlPane* open(DockingPaneManager* dockingManager,bool active=false);
    static VersionControlPane* getInstance(){return instance;}



public slots:
    void onProjectChanged(int i);
    void onUpdateCommit(void* repo,void* list);
    void onQueryCommit(int pos);
    void onCommitSelected(const QModelIndex &current, const QModelIndex &previous);

private:
    VersionControlPane(QWidget *parent = nullptr);

public:
    //event message list id
    static const QString M_UPLOAD;
    static const QString M_QUERY_COMMIT;
    static const QString M_QUERY_DIFF;

    static const QString PANE_ID;
    static const QString PANE_GROUP;

private:
    VersionControlPanePrivate* d;
    Ui::VersionControlPane *ui;
    static VersionControlPane* instance;
};
}

#endif // VERSIONCONTROLPANE_H
