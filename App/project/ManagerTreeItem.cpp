
#include <QList>
#include <QDebug>
#include "ManagerTreeItem.h"
namespace ady {

ManagerTreeData& ManagerTreeData::operator=(const ManagerTreeData& cls)
{
    if (this != &cls)
    {
        this->id = cls.id;
        this->name = cls.name;
        this->path = cls.path;
        this->role = cls.role;
    }
    return *this;
}

ManagerTreeData::ManagerTreeData()
{
    this->id = 0l;
    this->role = PROJECT;

}

ManagerTreeData::ManagerTreeData(Role role,long long id,QString name,QString path)
{
    this->role = role;
    this->id = id;
    this->name = name;
    this->path = path;
}



//////////////////////////////////////////////////////////////////////////////////////////
///  ManagerTreeItem::ManagerTreeItem
///
///

ManagerTreeItem::ManagerTreeItem(const ManagerTreeData &data, ManagerTreeItem *parent)
{
    m_parentItem = parent;
    m_itemData = data;
}

ManagerTreeItem::~ManagerTreeItem()
{
    qDeleteAll(m_childItems);
}

void ManagerTreeItem::updateData(ManagerTreeData &data)
{
    this->m_itemData = data;
}

void ManagerTreeItem::appendChild(ManagerTreeItem *item)
{
    m_childItems.append(item);
}

ManagerTreeItem *ManagerTreeItem::child(int row)
{
    return m_childItems.value(row);
}

void ManagerTreeItem::remove(int row)
{
    qDebug()<<"row:"<<row<<";size:"<<m_childItems.size();
    if(row<m_childItems.size()){
        m_childItems.removeAt(row);
    }
}

int ManagerTreeItem::childCount() const
{
    return m_childItems.count();
}


int ManagerTreeItem::columnCount() const
{
    return Max;
}

QVariant ManagerTreeItem::data(int column) const
{
    //return m_itemData.value(column);
    switch(column){
    case 0:
        return m_itemData.name;
    case 1:
        return m_itemData.path;
    }
}

ManagerTreeItem *ManagerTreeItem::parentItem()
{
    return m_parentItem;
}

int ManagerTreeItem::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<ManagerTreeItem*>(this));

    return 0;
}

void ManagerTreeItem::clearAll()
{
    this->m_childItems.clear();
}

int ManagerTreeItem::childIndex(ManagerTreeData::Role role,long long id)
{
    int size = this->childCount();
    for(int i=0;i<size;i++){
        ManagerTreeItem* item = this->child(i);
        if(item->equal(role,id)){
            return i;
        }
    }
    return -1;
}

bool ManagerTreeItem::equal(ManagerTreeData::Role role,long long id)
{
    return this->m_itemData.role == role && this->m_itemData.id == id ;
}

}
