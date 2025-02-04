#include "resource_manage_tree_view.h"
#include "resource_manager_model.h"
#include "resource_manager_model_item.h"
#include <QKeyEvent>
#include <QDebug>
namespace ady{
class ResourceManageTreeViewPrivate{
public:
    QModelIndex index;
};

ResourceManageTreeView::ResourceManageTreeView(QWidget* parent)
    :TreeView(parent)
{
    d = new ResourceManageTreeViewPrivate;
}

ResourceManageTreeView::~ResourceManageTreeView(){
    delete d;
}

void ResourceManageTreeView::editIndex(const QModelIndex& index){
    d->index = index;
    QTreeView::edit(index);
}

void ResourceManageTreeView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint){
    /*if(d->index.isValid()){
        auto item = static_cast<ResourceManagerModelItem*>(d->index.internalPointer());
        if(item->path().isEmpty()){
            static_cast<ResourceManagerModel*>(this->model())->removeItem(item);
        }
        d->index = {};
    }*/
    d->index = {};
    QAbstractItemView::closeEditor(editor,hint);
}


}
