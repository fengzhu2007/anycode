#ifndef TREEVIEW_H
#define TREEVIEW_H
#include "global.h"
#include <QTreeView>
#include <QDropEvent>
#include <QStringList>
#include <QMimeData>
namespace ady {
    class ANYENGINE_EXPORT TreeView : public QTreeView
    {
        Q_OBJECT
    public:

        explicit TreeView(QWidget* parent=nullptr);


        void addMimeType(QString mimeType);
        void setMimeTypes(QStringList mimeTypes);
        void setSupportDropFile(bool res);



    signals:
        void testA();
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
#endif // TREEVIEW_H
