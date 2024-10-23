#ifndef FILE_TRANSFER_PANE_H
#define FILE_TRANSFER_PANE_H
#include "global.h"
#include "docking_pane.h"
#include "core/event_bus/subscriber.h"
#include "core/event_bus/event_data.h"
namespace Ui {
class FileTransferPane;
}
namespace ady{

class FileTransferPanePrivate;
class ANYENGINE_EXPORT FileTransferPane : public DockingPane , public Subscriber
{
    Q_OBJECT

public:
    ~FileTransferPane();
    void initView();
    virtual QString id() override;
    virtual QString group() override;
    virtual bool onReceive(Event* e) override;//event bus receive callback

    void addJob(UploadData* data);
    static FileTransferPane* open(DockingPaneManager* dockingManager,bool active=false);

    static FileTransferPane* make(DockingPaneManager* dockingManager,const QJsonObject& data);
public slots:
    void onContextMenu(const QPoint& pos);

private:
    FileTransferPane(QWidget *parent = nullptr);

public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;
private:
    Ui::FileTransferPane *ui;
    FileTransferPanePrivate* d;
    static FileTransferPane* instance;



};
}
#endif // FILE_TRANSFER_PANE_H
