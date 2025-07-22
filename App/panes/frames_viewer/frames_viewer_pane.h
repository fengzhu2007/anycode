#ifndef FRAMES_VIEWER_PANE_H
#define FRAMES_VIEWER_PANE_H

#include <QWidget>
#include <docking_pane.h>
namespace Ui {
class FramesViewerPane;
}
namespace ady{
class FramesViewerPanePrivate;
class FramesViewerPane : public DockingPane
{
    Q_OBJECT

public:
    explicit FramesViewerPane(QWidget *parent = nullptr);
    ~FramesViewerPane();
    void initView();
    virtual QString id() override;
    virtual QString group() override;
    //virtual bool onReceive(Event* e) override;//event bus receive callback

    static FramesViewerPane* open(DockingPaneManager* dockingManager,bool active=false);
    static FramesViewerPane* make(DockingPaneManager* dockingManager,const QJsonObject& data);


protected:
    virtual void resizeEvent(QResizeEvent* e) override;

public slots:
    void onLoad();
    void onDoubleClicked(const QModelIndex& index);
    void onActionTriggered();
    void onFrameChange();
    void onListContextMenu(const QPoint &pos);
    void onIntervalChanged(int interval);
public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;


private:
    Ui::FramesViewerPane *ui;
    FramesViewerPanePrivate* d;
    static int SN;
};
}
#endif // FRAMES_VIEWER_PANE_H
