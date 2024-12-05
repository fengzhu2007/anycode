#ifndef GRID_VIEW_H
#define GRID_VIEW_H
#include "global.h"
#include <QListView>
#include <QDropEvent>
#include <QStringList>
#include <QMimeData>
namespace ady{
class ANYENGINE_EXPORT GridView : public QListView
{
    Q_OBJECT
public:
    explicit GridView(QWidget* parent=nullptr);


    void addMimeType(const QString& mimeType);
    void setMimeTypes(const QStringList& mimeTypes);
    void setSupportDropFile(bool res);


signals:
    void dropFinished(const QMimeData* data);

protected:
    bool dropMimeData(const QMimeData* data);
    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;


protected:
    QStringList supportMimeTypes;
    bool supportDropFile = false;

};
}
#endif // GRID_VIEW_H
