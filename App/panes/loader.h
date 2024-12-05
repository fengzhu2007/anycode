#ifndef PANELOADER_H
#define PANELOADER_H
#include <QString>
#include <QJsonObject>
namespace ady{
class DockingPane;
class DockingPaneManager;
class PaneLoader
{
public:
    static DockingPane* init(DockingPaneManager* dockingManager,const QString& group,const QJsonObject& data);
    static DockingPane* open(DockingPaneManager* dockingManager,const QString& group,const QJsonObject& data);
private:
    PaneLoader();
};
}
#endif // PANELOADER_H
