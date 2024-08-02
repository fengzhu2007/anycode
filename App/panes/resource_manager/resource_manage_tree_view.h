#ifndef RESOURCEMANAGETREEVIEW_H
#define RESOURCEMANAGETREEVIEW_H
#include "global.h"
#include <QTreeView>
namespace ady{
class ResourceManageTreeViewPrivate;
class ANYENGINE_EXPORT ResourceManageTreeView : public QTreeView
{
    Q_OBJECT
public:
    ResourceManageTreeView(QWidget* parent);
    ~ResourceManageTreeView();
    void editIndex(const QModelIndex& index);
protected:
    virtual void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) override;
private:
    ResourceManageTreeViewPrivate* d;
};
}

#endif // RESOURCEMANAGETREEVIEW_H
