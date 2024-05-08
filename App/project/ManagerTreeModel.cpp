#include "ManagerTreeModel.h"
#include "ManagerTreeItem.h"
#include "storage/SiteStorage.h"
#include "storage/GroupStorage.h"
#include "storage/ProjectStorage.h"
#include <QDebug>
namespace ady {


ManagerTreeModel::ManagerTreeModel(QList<ManagerTreeItem*> lists , QObject *parent)
    :QAbstractItemModel(parent)
{
    ManagerTreeData data;
    rootItem = new ManagerTreeItem(data,nullptr);
    this->setData(lists);
}

ManagerTreeModel::~ManagerTreeModel()
{
    delete rootItem;
}

QVariant ManagerTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    ManagerTreeItem *item = static_cast<ManagerTreeItem*>(index.internalPointer());

    //qDebug()<<"item:"<<item;

    return item->data(index.column());
}

Qt::ItemFlags ManagerTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant ManagerTreeModel::headerData(int section, Qt::Orientation orientation,int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex ManagerTreeModel::index(int row, int column,const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ManagerTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<ManagerTreeItem*>(parent.internalPointer());

    ManagerTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex ManagerTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    ManagerTreeItem *childItem = static_cast<ManagerTreeItem*>(index.internalPointer());
    ManagerTreeItem *parentItem = childItem->parentItem();
    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int ManagerTreeModel::rowCount(const QModelIndex &parent) const
{
    ManagerTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<ManagerTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int ManagerTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<ManagerTreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

void ManagerTreeModel::setData(QList<ManagerTreeItem*> lists)
{
    rootItem->clearAll();
    for(int i=0;i<lists.length();i++){
        rootItem->appendChild(lists[i]);
    }
}

void ManagerTreeModel::notifyDataChanged(ManagerTreeItem* item)
{
    int row = item->row();
    QModelIndex lt = createIndex(row,0,item);
    QModelIndex rb = createIndex(row,1,item);
    emit dataChanged(lt, rb,{ Qt::DisplayRole });
}

void ManagerTreeModel::appendRow(SiteRecord* record)
{
    //QModelIndex rootIndex = this.mo
    ManagerTreeItem* rootItem = this->getRootItem();
    //QModelIndex rootIndex = this->index(0,0);
    int index = rootItem->childIndex(ManagerTreeData::PROJECT,record->pid);
    if(index>=0){
        QModelIndex projectIndex = this->index(index,0);
        ManagerTreeItem* projectItem = rootItem->child(index);
        index = projectItem->childIndex(ManagerTreeData::GROUP,record->groupid);
        if(index>=0){
            QModelIndex groupIndex = this->index(index,0,projectIndex);
            ManagerTreeItem* groupItem = projectItem->child(index);
            index = groupItem->childIndex(ManagerTreeData::SITE,record->id);
            ManagerTreeData data(ManagerTreeData::SITE,record->id,record->name,record->path);
            if(index>=0){
                //update
                ManagerTreeItem* siteItem = groupItem->child(index);
                siteItem->updateData(data);
                this->notifyDataChanged(siteItem);
            }else{
                //append
                int position = groupItem->childCount();
                beginInsertRows(groupIndex,position,position + 1);
                ManagerTreeItem* siteItem = new ManagerTreeItem(data,groupItem);
                groupItem->appendChild(siteItem);
                endInsertRows();
            }
        }
    }
}

void ManagerTreeModel::appendRow(int pid,GroupRecord* record)
{
    ManagerTreeItem* rootItem = this->getRootItem();
    //QModelIndex rootIndex = this->index(0,0);
    int index = rootItem->childIndex(ManagerTreeData::PROJECT,pid);
    if(index>=0){
        QModelIndex projectIndex = this->index(index,0);
        ManagerTreeItem* projectItem = rootItem->child(index);
        index = projectItem->childIndex(ManagerTreeData::GROUP,record->id);

        ManagerTreeData data(ManagerTreeData::GROUP,record->id,record->name,"");
        if(index>=0){
            //update
            ManagerTreeItem* groupItem = projectItem->child(index);
            groupItem->updateData(data);
            this->notifyDataChanged(groupItem);
        }else{
            //append
            int position = projectItem->childCount();
            beginInsertRows(projectIndex,position,position + 1);
            ManagerTreeItem* groupItem = new ManagerTreeItem(data,projectItem);
            projectItem->appendChild(groupItem);
            endInsertRows();
        }
    }
}

void ManagerTreeModel::appendRow(ProjectRecord* record)
{
    int index = rootItem->childIndex(ManagerTreeData::PROJECT,record->id);
    ManagerTreeData data(ManagerTreeData::PROJECT,record->id,record->name,record->path);
    QModelIndex rootIndex ;

    if(index>=0){
        //update
        ManagerTreeItem* projectItem = rootItem->child(index);
        projectItem->updateData(data);
        this->notifyDataChanged(projectItem);
    }else{
        //append
        int position = rootItem->childCount();
        beginInsertRows(rootIndex,position,position + 1);
        ManagerTreeItem* projectItem = new ManagerTreeItem(data,rootItem);
        rootItem->appendChild(projectItem);
        endInsertRows();
    }
}

void ManagerTreeModel::removeRow(SiteRecord* record)
{
    ManagerTreeItem* rootItem = this->getRootItem();
    //QModelIndex rootIndex = this->index(0,0);
    int index = rootItem->childIndex(ManagerTreeData::PROJECT,record->pid);
    if(index>=0){
        //QModelIndex projectIndex = this->index(index,0,rootIndex);
        ManagerTreeItem* projectItem = rootItem->child(index);
        //qDebug()<<"project point:"<<((ManagerTreeItem* )(projectIndex.internalPointer()))->dataId();
        index = projectItem->childIndex(ManagerTreeData::GROUP,record->groupid);
        if(index>=0){
            //qDebug()<<"projectid:"<<projectItem->dataId()<<";index:"<<index;
            //QModelIndex groupIndex = this->index(index,0,projectIndex);

            ManagerTreeItem* groupItem = projectItem->child(index);
            QModelIndex groupIndex = createIndex(index,0,groupItem);
            index = groupItem->childIndex(ManagerTreeData::SITE,record->id);
            //ManagerTreeData data(ManagerTreeData::SITE,record->id,record->name,record->path);
            if(index>=0){
                //remove

               // qDebug()<<"index:"<<groupIndex.row();
               // qDebug()<<"point:"<<((ManagerTreeItem* )(groupIndex.internalPointer()))->dataId()<<";p1:"<<groupItem->dataId();
                ManagerTreeItem* siteItem = groupItem->child(index);
                beginRemoveRows(groupIndex,index,index);
                groupItem->remove(index);
                endRemoveRows();
                delete  siteItem;
            }
        }
    }
}




}
