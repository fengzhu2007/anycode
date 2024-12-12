#include "select_tree_model.h"
#include "storage/project_storage.h"
#include "storage/site_storage.h"
#include <QFile>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

namespace ady{

SelectTreeModel::SelectTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_root = new SelectTreeModelItem();
}


SelectTreeModel::~SelectTreeModel(){
    delete m_root;
}

QVariant SelectTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // FIXME: Implement me!
    if(section==Title && role==Qt::DisplayRole){
        return tr("Title");
    }
    return QVariant();
}

QModelIndex SelectTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    // FIXME: Implement me!
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    SelectTreeModelItem *parentItem;

    if (!parent.isValid())
        parentItem = m_root;
    else
        parentItem = static_cast<SelectTreeModelItem*>(parent.internalPointer());

    SelectTreeModelItem *childItem = parentItem->m_children.at(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex SelectTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    SelectTreeModelItem *childItem = static_cast<SelectTreeModelItem*>(index.internalPointer());
    if(childItem==m_root || childItem==nullptr){
        return QModelIndex();
    }

    SelectTreeModelItem *parentItem = childItem->m_parent;
    if (parentItem == m_root || parentItem==nullptr)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

int SelectTreeModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_root->m_children.count();

    SelectTreeModelItem* parentItem = static_cast<SelectTreeModelItem*>(parent.internalPointer());
    if(parentItem!=nullptr){
        return parentItem->m_children.count();
    }else{
        return 0;
    }
}

int SelectTreeModel::columnCount(const QModelIndex &parent) const
{
    return Max;
}

bool SelectTreeModel::setData(const QModelIndex &index, const QVariant &value, int role){
    if (!index.isValid()) return false;

    SelectTreeModelItem *node = static_cast<SelectTreeModelItem *>(index.internalPointer());
    if (role == Qt::CheckStateRole) {
        bool checked = (value.toInt() == Qt::Checked);
        node->checkedAll(checked);

        if(node->m_parent!=nullptr && node->m_parent!=m_root){
            //update parent
            //auto parent = node->m_parent;
            auto parentIndex = createIndex(node->m_parent->row(),0,node->m_parent);
            emit dataChanged(parentIndex, parentIndex, {Qt::CheckStateRole});

        }
        emit dataChanged(index, index, {Qt::CheckStateRole});
        //update children
        if(node->m_children.size()>0){
            auto topIndex = createIndex(0,0,node->m_children.first());
            auto bottomIndex = createIndex(node->m_children.size() - 1,0,node->m_children.last());
            emit dataChanged(topIndex, bottomIndex, {Qt::CheckStateRole});
        }

        return true;
    }
    return false;
}

QVariant SelectTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if(role==Qt::DisplayRole){
        auto item = static_cast<SelectTreeModelItem*>(index.internalPointer());
        int col = index.column();
        if(col==Title){
            return item->m_title;
        }
    }else if (role == Qt::CheckStateRole) {
        auto item = static_cast<SelectTreeModelItem*>(index.internalPointer());
        return item->state();
    }
    return QVariant();
}

Qt::ItemFlags SelectTreeModel::flags(const QModelIndex &index) const{

    if (!index.isValid())
        return 0;
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    flags |= Qt::ItemIsUserCheckable;
    return flags;
}

void SelectTreeModel::checkedAll(bool state){
    m_root->checkedAll(state);
    if(m_root->m_children.size()>0){
        auto topIndex = createIndex(0,0,m_root->m_children.first());
        auto bottomIndex = createIndex(m_root->m_children.size() - 1,0,m_root->m_children.last());
        emit dataChanged(topIndex, bottomIndex, {Qt::CheckStateRole});
    }
}


QString SelectTreeModel::readFile(const QString& path){
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return tr("Open file failed");
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(in.readAll().toUtf8(),&error);
    if(error.error==QJsonParseError::NoError){
        QJsonObject json = doc.object();
        int version = json.find("version")->toInt(1);
        const QString name = json.find("name")->toString();
        QJsonArray data = json.find(("data"))->toArray();
        if(data.size()==0){
            return {"Invalid content"};
        }
        this->beginResetModel();
        qDeleteAll(m_root->m_children);//clear old data
        for(auto one:data){
            QJsonObject obj = one.toObject();
            int type = obj.find("type")->toInt(0);
            auto item = obj.find("item")->toObject();
            if(item.isEmpty()){
                continue;
            }
            if(type==SelectTreeModelItem::Project){
                //project
                ProjectRecord r;
                r.fromJson(item);
                if(r.id>0){
                    auto proj = new SelectTreeModelItem(SelectTreeModelItem::Project,r.id,r.name,m_root);
                    auto list = item.find("list")->toArray();
                    for(auto s:list){
                        SiteRecord site;
                        site.fromJson(s.toObject());
                        if(site.id>0){
                            auto child = new SelectTreeModelItem(SelectTreeModelItem::Site,site.id,site.name,proj);
                            child->m_state = Qt::Checked;
                            proj->m_children<<child;
                        }
                    }
                    m_root->m_children<<proj;
                }
            }else if(type==SelectTreeModelItem::Site){
                //no project site
                SiteRecord site;
                site.fromJson(item);
                if(site.id>0){
                    auto child = new SelectTreeModelItem(SelectTreeModelItem::Site,site.id,site.name,m_root);
                    child->m_state = Qt::Checked;
                    m_root->m_children<<child;
                }
            }
        }
        this->endResetModel();
        return {};
    }else{
        qDebug()<<"json parse error:"<<error.errorString()<<";offset:"<<error.offset;
        return error.errorString();
    }
}



}
