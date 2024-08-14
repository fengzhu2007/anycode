#ifndef FIND_REPLACE_PANE_H
#define FIND_REPLACE_PANE_H

#include "global.h"
#include "docking_pane.h"
#include "core/event_bus/subscriber.h"
#include "search_result_model.h"
namespace Ui {
class FindReplacePane;
}
namespace ady{

class FindReplacePanePrivate;
class ANYENGINE_EXPORT FindReplacePane : public DockingPane , public Subscriber
{
    Q_OBJECT

public:
    ~FindReplacePane();
    void initView();
    virtual QString id() override;
    virtual QString group() override;
    virtual bool onReceive(Event* e) override;//event bus receive callback

    void runSearch(const QString& text,const QString& scope,int flags,const QString& filter,const QString& exclusion);
    void runReplace(const QString& before,const QString& after,const QString& scope,int flags, const QString& filter,const QString& exclusion);



    static FindReplacePane* open(DockingPaneManager* dockingManager,bool active=false);
public slots:
    void stop();
    void onSearchEnd();
    void onSearchResult(QList<SearchResultItem>* list,int matchFileCount,int searchFileCount);
    void onSearchItemActivated(const QModelIndex& index);
    void onActionTriggered();
    void onReplaceOpendFiles(const QStringList& list,const QString& before,const QString& after,int flags,int replaceCount,int replaceFiles);
    void onReplaceAllFinished(int result);



protected:
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    explicit FindReplacePane(QWidget *parent = nullptr);

public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;

private:
    Ui::FindReplacePane *ui;
    FindReplacePanePrivate *d;
    static FindReplacePane* instance;

};

}
#endif // FIND_REPLACE_PANE_H
