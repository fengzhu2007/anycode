#include "resource_manager_model.h"
#include "resource_manager_model_item.h"
#include "resource_manage_icon_provider.h"
namespace ady{

class ResourceManagerModelPrivate{
public:
    ResourceManagerModelItem* root;
    ResourceManageIconProvider* iconProvider;
};


ResourceManagerModel::ResourceManagerModel(QObject *parent)
    : QAbstractItemModel{parent}
{
    d = new ResourceManagerModelPrivate();
    d->iconProvider = ResourceManageIconProvider::getInstance();
    d->root = new ResourceManagerModelItem();
}

ResourceManagerModel::~ResourceManagerModel(){
    ResourceManageIconProvider::destory();
    delete d;
}


QVariant ResourceManagerModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();
    if(role == Qt::DisplayRole){
        ResourceManagerModelItem* item = static_cast<ResourceManagerModelItem*>(index.internalPointer());
        return item->data(index.column());
    }else if(role == Qt::DecorationRole){
        if(index.column() == Name){
            ResourceManagerModelItem* item = static_cast<ResourceManagerModelItem*>(index.internalPointer());
            return d->iconProvider->icon(item);
        }else{
            return QVariant();
        }
    }else{
        return QVariant();
    }
}

Qt::ItemFlags ResourceManagerModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant ResourceManagerModel::headerData(int section, Qt::Orientation orientation,int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole){
        switch(section){
        case Name:
            return tr("Name");
            break;
        }
    }
    return QVariant();
}

QModelIndex ResourceManagerModel::index(int row, int column,const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
            return QModelIndex();

    ResourceManagerModelItem *parentItem;

    if (!parent.isValid())
            parentItem = d->root;
    else
            parentItem = static_cast<ResourceManagerModelItem*>(parent.internalPointer());

    ResourceManagerModelItem *childItem = parentItem->childAt(row);
    if (childItem)
            return createIndex(row, column, childItem);
    else
            return QModelIndex();
}

QModelIndex ResourceManagerModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    ResourceManagerModelItem *childItem = static_cast<ResourceManagerModelItem*>(index.internalPointer());
    ResourceManagerModelItem *parentItem = childItem->parent();

    if (parentItem == d->root || parentItem==nullptr)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int ResourceManagerModel::rowCount(const QModelIndex &parent) const {
    ResourceManagerModelItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid()){
        parentItem = d->root;
        return parentItem->childrenCount();
    }else{
        parentItem = static_cast<ResourceManagerModelItem*>(parent.internalPointer());
        if(parentItem!=nullptr){
            return parentItem->childrenCount();
        }else{
            return 0;
        }
    }
}

int ResourceManagerModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return Max;
}

}
