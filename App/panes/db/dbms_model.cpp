#include "dbms_model.h"
#include "storage/db_storage.h"
#include "db_driver.h"
#include <QFileIconProvider>
#include <QFileInfo>
#include <QIcon>

namespace ady{




class DBMSModelItemPrivate{
public:
    DBMSModelItem::Type type;
    DBDriver::Type itemType;
    long long id;
    bool expanded=false;
    bool loading = false;
    DBMSModelItem* parent=nullptr;
    QString name;
    QString driver;
    QList<DBMSModelItem*> children;
    bool status = false;
};

DBMSModelItem::DBMSModelItem(){
    d = new DBMSModelItemPrivate();
    d->type = Solution;
}

DBMSModelItem::DBMSModelItem(Type type,long long id,const QString& driver,const QString& name,DBMSModelItem* parent){
    d = new DBMSModelItemPrivate();
    d->type = type;
    d->id = id;
    d->driver = driver;
    d->name = name;
    d->parent = parent;
}

DBMSModelItem::DBMSModelItem(Type type,const QString& name,DBMSModelItem* parent){
    d = new DBMSModelItemPrivate();
    d->type = type;
    d->id = 0l;
    d->name = name;
    d->parent = parent;
}

DBMSModelItem::DBMSModelItem(Type type,int itemType,const QString& name,DBMSModelItem* parent){
    d = new DBMSModelItemPrivate();
    d->type = type;
    d->itemType = static_cast<DBDriver::Type>(itemType);
    d->id = 0l;
    d->name = name;
    d->parent = parent;
}


DBMSModelItem::~DBMSModelItem(){
    qDeleteAll(d->children);
    delete d;
}

int DBMSModelItem::childrenCount(){
    return d->children.count();
}

int DBMSModelItem::row(){
    int i = -1;
    if(d->parent!=nullptr){
        for(auto one:d->parent->d->children){
            i+=1;
            if(one==this){
                break;
            }
        }
    }
    return i;
}

void DBMSModelItem::appendItems(QList<DBMSModelItem*> items){
    for(auto one:items){
        this->appendItem(one);
    }
}

void DBMSModelItem::appendItem(DBMSModelItem* item){
    d->children.push_back(item);
}

void DBMSModelItem::insertItem(int row,DBMSModelItem* item){
    d->children.insert(row,item);
}

DBMSModelItem* DBMSModelItem::parent(){
    return d->parent;
}

DBMSModelItem* DBMSModelItem::childAt(int i){
    return d->children.at(i);
}

DBMSModelItem* DBMSModelItem::take(int i){
    return d->children.takeAt(i);
}

QList<DBMSModelItem*> DBMSModelItem::takeAll(){
    auto list = d->children;
    d->children.clear();
    return list;
}

DBMSModelItem::Type DBMSModelItem::type(){
    return d->type;
}

QString DBMSModelItem::name() const{
    return d->name;
}

long long DBMSModelItem::id() const {
    return d->id;
}

long long DBMSModelItem::pid() const{
    if(d->type==DBMSModelItem::Connection){
        return d->id;
    }else if(d->type==DBMSModelItem::Solution){
        return 0l;
    }else{
        return d->parent->pid();
    }
}

bool DBMSModelItem::status() const{
    return d->status;
}

int DBMSModelItem::itemType()const{
    return d->itemType;
}

void DBMSModelItem::setName(const QString& name){
    d->name = name;
}

void DBMSModelItem::setStatus(bool status){
    d->status = status;
}

bool DBMSModelItem::expanded(){
    return d->expanded;
}

void DBMSModelItem::setExpanded(bool expanded){
    d->expanded = expanded;
}

bool DBMSModelItem::isLoading(){
    return d->loading;
}

void DBMSModelItem::setLoading(bool state){
    d->loading = state;
}

class DBMSModelPrivate{
public:
    DBMSModelPrivate():databaseIcon(QString::fromUtf8(":/Resource/icons/Database_16x.svg")),tableIcon(":/Resource/icons/Table_16x.svg"),viewIcon(":/Resource/icons/View_16x.svg"),itemGroupIcon(":/Resource/icons/DatabaseTableGroup_16x.svg"),sqliteIcon(":/Resource/icons/sqlite.svg"){
        sqliteDisabledIcon.addPixmap(sqliteIcon.pixmap(16,16,QIcon::Disabled));
        sqliteIcon.addPixmap(sqliteIcon.pixmap(16,16),QIcon::Active);
        databaseDisabledIcon.addPixmap(databaseIcon.pixmap(16,16,QIcon::Disabled));
    }
    DBMSModelItem* root = nullptr;
    QIcon databaseIcon;
    QIcon tableIcon;
    QIcon viewIcon;
    QIcon itemGroupIcon;
    QIcon sqliteIcon;
    QIcon sqliteDisabledIcon;
    QIcon databaseDisabledIcon;
    QMap<QString,QString> addons;
    QFileIconProvider* provider;
};


DBMSModel::DBMSModel(QObject *parent)
    : QAbstractItemModel{parent}
{
    d = new DBMSModelPrivate;
    d->root = new DBMSModelItem();
    d->provider = new QFileIconProvider();


}

DBMSModel::~DBMSModel(){
    delete d->provider;
    delete d->root;
    delete d;
}

// Header:
QVariant DBMSModel::headerData(int section, Qt::Orientation orientation, int role ) const {
    if(role==Qt::DisplayRole){
        if(section==Name){
            return tr("Name");
        }
    }
    return {};
}
// Basic functionality:
QModelIndex DBMSModel::index(int row, int column,const QModelIndex &parent ) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    DBMSModelItem *parentItem;

    if (!parent.isValid())
        parentItem = d->root;
    else
        parentItem = static_cast<DBMSModelItem*>(parent.internalPointer());

    DBMSModelItem *childItem = parentItem->childAt(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex DBMSModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    DBMSModelItem *childItem = static_cast<DBMSModelItem*>(index.internalPointer());
    if(childItem==d->root || childItem==nullptr){
        return QModelIndex();
    }

    DBMSModelItem *parentItem = childItem->parent();
    if (parentItem == d->root || parentItem==nullptr)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

int DBMSModel::rowCount(const QModelIndex &parent) const {
    if (!parent.isValid())
        return d->root->childrenCount();

    DBMSModelItem* parentItem = static_cast<DBMSModelItem*>(parent.internalPointer());
    if(parentItem!=nullptr){
        return parentItem->childrenCount();
    }else{
        return 0;
    }
}

int DBMSModel::columnCount(const QModelIndex &parent) const{
    return Max;
}

QVariant DBMSModel::data(const QModelIndex &index, int role) const{
    if (!index.isValid())
        return QVariant();
    if(role == Qt::DisplayRole){
        DBMSModelItem* item = static_cast<DBMSModelItem*>(index.internalPointer());
        int col = index.column();
        DBMSModelItem::Type type = item->type();
        if(col==Name){
            if(item->isLoading()){
                return QString("%1(%2)").arg(item->name(),tr("Loading"));
            }else{
                return item->name();
            }
        }
    }else if(role == Qt::DecorationRole){
        if(index.column() == Name){
            DBMSModelItem* item = static_cast<DBMSModelItem*>(index.internalPointer());
            //DownloadFolder
            DBMSModelItem::Type type = item->type();
            if(type==DBMSModelItem::Connection){
                return item->status()?d->sqliteIcon:d->sqliteDisabledIcon;
            }else if(type==DBMSModelItem::DatabaseItem){
                return item->status()?d->databaseIcon:d->databaseDisabledIcon;
            }else if(type==DBMSModelItem::ItemType){
                return d->itemGroupIcon;
            }else if(type==DBMSModelItem::Table){
                return d->tableIcon;
            }else if(type==DBMSModelItem::View){
                return d->viewIcon;
            }
        }else{
            return QVariant();
        }
    }else if(role==Qt::EditRole){
         if(index.column()==Name){
            DBMSModelItem* item = static_cast<DBMSModelItem*>(index.internalPointer());
            return item->name();
        }
    }
    return QVariant();
}

bool DBMSModel::hasChildren(const QModelIndex &parent)const{
    if(parent.isValid()){
        auto item = static_cast<DBMSModelItem*>(parent.internalPointer());
        if(item!=nullptr){
            switch(item->type()){
            case DBMSModelItem::Connection:
            case DBMSModelItem::DatabaseItem:
                return item->status();
                break;
            case DBMSModelItem::Solution:
            case DBMSModelItem::ItemType:
                return true;
            case DBMSModelItem::Table:
            case DBMSModelItem::View:
                return false;
            }
        }
    }
    return true;
}

bool DBMSModel::setData(const QModelIndex &index, const QVariant &value, int role){
    if(role==Qt::EditRole){
        if(index.column() ==Name ){
            QString name = value.toString();
            DBMSModelItem* item = static_cast<DBMSModelItem*>(index.internalPointer());
            if(!name.isEmpty() && name!=item->name()){
                /*const QString src = item->path();
                const QString dest = item->parent()->path() + name;
                emit rename(src,dest,item->type());*/
                emit rename(item,name);
            }
        }
    }
    return QAbstractItemModel::setData(index,value,role);
}

Qt::ItemFlags DBMSModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    flags = flags | Qt::ItemIsDragEnabled ;
    if(index.column()==Name){
        DBMSModelItem* item = static_cast<DBMSModelItem*>(index.internalPointer());
        DBMSModelItem::Type type = item->type();
        if(type==DBMSModelItem::Table || type==DBMSModelItem::View){
            flags |= Qt::ItemIsEditable;
        }
    }
    return flags;
}

void DBMSModel::appendItem(DBMSModelItem* parent,DBMSModelItem* item){
    int position = parent->childrenCount();
    QModelIndex parentIndex;
    if(parent!=d->root){
        parentIndex = createIndex(parent->row(),0,parent);
    }
    beginInsertRows(parentIndex,position,position);
    parent->appendItem(item);
    endInsertRows();
}

void DBMSModel::setDatasource(const QList<DBRecord>& list){
    beginResetModel();
    delete d->root;
    d->root = new DBMSModelItem();
    for(auto one:list){
        auto item = new DBMSModelItem(DBMSModelItem::Connection,one.id,one.driver,one.name,d->root);
        d->root->appendItem(item);
    }
    endResetModel();
}

void DBMSModel::addConnection(long long id,const QString& driver,const QString& name){
    auto item = new DBMSModelItem(DBMSModelItem::Connection,id,driver,name,d->root);
    this->appendItem(d->root,item);
}


void DBMSModel::updateConnection(long long id,const QString& name){
    int total = d->root->childrenCount();
    for(int i=0;i<total;i++){
        auto item = d->root->childAt(i);
        if(item->id()==id){
            item->setName(name);
            changeItem(item);
            return ;
        }
    }
}


void DBMSModel::updateConnection(long long id,bool status){
    int total = d->root->childrenCount();
    for(int i=0;i<total;i++){
        auto item = d->root->childAt(i);
        if(item->id()==id){
            item->setStatus(status);
            changeItem(item);
            if(status==false){
                //remove all children
                this->removeChildren(item);
            }
            return ;
        }
    }
}

void DBMSModel::removeConnection(long long id){
    int total = d->root->childrenCount();
    for(int i=0;i<total;i++){
        auto item = d->root->childAt(i);
        if(item->id()==id){
            this->removeItem(item);
            return ;
        }
    }
}

void DBMSModel::removeChildren(DBMSModelItem* item){
    auto index = createIndex(item->row(),0,item);
    beginRemoveRows(index,0,item->childrenCount() - 1);
    auto list = item->takeAll();
    endRemoveRows();
    qDeleteAll(list);
}

void DBMSModel::addDatabase(const QStringList& list,DBMSModelItem* parent){
    if(list.size()==0){
        return ;
    }
    int position = parent->childrenCount();
    auto parentIndex = createIndex(parent->row(),0,parent);
    beginInsertRows(parentIndex,position,position);
    for(auto one:list){
        auto item = new DBMSModelItem(DBMSModelItem::DatabaseItem,one,parent);
        parent->appendItem(item);
    }
    endInsertRows();
}

void DBMSModel::addType(const QList<QPair<int,QString>> list,DBMSModelItem* parent){
    if(list.size()==0){
        return ;
    }
    int position = parent->childrenCount();
    auto parentIndex = createIndex(parent->row(),0,parent);
    beginInsertRows(parentIndex,position,position);
    for(auto one:list){
        auto item = new DBMSModelItem(DBMSModelItem::ItemType,one.first,one.second,parent);
        parent->appendItem(item);
    }
    endInsertRows();
}

void DBMSModel::addTable(const QStringList& list,DBMSModelItem* parent){
    if(list.size()==0){
        return ;
    }
    int position = parent->childrenCount();
    auto parentIndex = createIndex(parent->row(),0,parent);
    beginInsertRows(parentIndex,position,position);
    for(auto one:list){
        auto item = new DBMSModelItem(DBMSModelItem::Table,one,parent);
        parent->appendItem(item);
    }
    endInsertRows();
}

void DBMSModel::addView(const QStringList& list,DBMSModelItem* parent){
    if(list.size()==0){
        return ;
    }
    int position = parent->childrenCount();
    auto parentIndex = createIndex(parent->row(),0,parent);
    beginInsertRows(parentIndex,position,position);
    for(auto one:list){
        auto item = new DBMSModelItem(DBMSModelItem::View,one,parent);
        parent->appendItem(item);
    }
    endInsertRows();
}

void DBMSModel::updateDatabase(const QModelIndex& index,bool status){
    auto item = static_cast<DBMSModelItem*>(index.internalPointer());
    item->setStatus(status);
    changeItem(item);
    if(status==false){
        this->removeChildren(item);//remove all children
    }
}

void DBMSModel::removeItem(DBMSModelItem* item){
    //QMutexLocker locker(&d->mutex);
    auto parent = item->parent();
    int position = item->row();
    QModelIndex parentIndex;
    if(parent!=d->root){
        parentIndex = createIndex(parent->row(),0,parent);
    }
    beginRemoveRows(parentIndex,position,position);
    auto child = parent->take(position);
    endRemoveRows();
    //delete child;
    delete child;
}



DBMSModelItem* DBMSModel::rootItem(){
    return d->root;
}

void DBMSModel::changeItem(DBMSModelItem* item){
    QModelIndex index = createIndex(item->row(),0,item);
    emit dataChanged(index,index,QVector<int>{Qt::DisplayRole});
}

}
