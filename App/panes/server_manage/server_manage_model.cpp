#include "server_manage_model.h"
#include "panes/resource_manager/resource_manager_model.h"
#include "panes/resource_manager/resource_manager_model_item.h"
#include "storage/site_storage.h"

#include "network/network_manager.h"

#include <QFileIconProvider>
#include <QFileInfo>
#include <QIcon>

namespace ady{

class FileItemSorting final{
public:
    FileItemSorting(ServerManageModel::Column col,bool asc){
        this->col = col;
        this->asc = asc;
    }
    bool operator()(FileItem& a,FileItem& b)
    {
        if(this->asc){
            if(a.type!=b.type){
                return a.type>b.type;
            }
            if(this->col==ServerManageModel::Name){
                return a.name.compare(b.name,Qt::CaseInsensitive)<0;
            }

        }else{
            if(a.type!=b.type){
                return a.type<b.type;
            }
            if(this->col==ServerManageModel::Name){
                return a.name.compare(b.name,Qt::CaseInsensitive)>=0;
            }
        }
        return true;
    }

    void setCol(int col)
    {
        this->col = col;
    }
    void setAsc(bool asc){
        this->asc = asc;
    }
private:
    bool asc;
    int col;
};





class ServerManageModelItemPrivate{
public:
    ServerManageModelItem::Type type;
    long long id;
    bool expanded=false;
    bool loading = false;
    ServerManageModelItem* parent=nullptr;
    QString name;
    QString path;
    QList<ServerManageModelItem*> children;
    FileItem* data = nullptr;
};

ServerManageModelItem::ServerManageModelItem(){
    d = new ServerManageModelItemPrivate();
    d->type = Solution;
}

ServerManageModelItem::ServerManageModelItem(Type type,long long id,const QString& name,ServerManageModelItem* parent){
    d = new ServerManageModelItemPrivate();
    d->type = type;
    d->id = id;
    d->name = name;
    d->parent = parent;
}

ServerManageModelItem::ServerManageModelItem(Type type,long long id,const QString& name,const QString& path,ServerManageModelItem* parent){
    d = new ServerManageModelItemPrivate();
    d->type = type;
    d->id = id;
    d->name = name;
    d->path = path;
    d->parent = parent;
}


ServerManageModelItem::ServerManageModelItem(const FileItem& item,ServerManageModelItem* parent){
    d = new ServerManageModelItemPrivate();
    d->type = item.type==FileItem::Dir?Folder:File;
    d->id = 0l;
    d->name = item.name;
    d->path = item.path;
    d->parent = parent;
    d->data = new FileItem;
    *d->data = item;
}

ServerManageModelItem::~ServerManageModelItem(){
    if(d->data!=nullptr){
        delete d->data;
    }
    qDeleteAll(d->children);
    delete d;
}

int ServerManageModelItem::childrenCount(){
    return d->children.count();
}

int ServerManageModelItem::row(){
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

void ServerManageModelItem::appendItems(QList<ServerManageModelItem*> items){
    for(auto one:items){
        this->appendItem(one);
    }
}

void ServerManageModelItem::appendItem(ServerManageModelItem* item){
    d->children.push_back(item);
}
ServerManageModelItem* ServerManageModelItem::parent(){
    return d->parent;
}

ServerManageModelItem* ServerManageModelItem::childAt(int i){
    return d->children.at(i);
}

QList<ServerManageModelItem*> ServerManageModelItem::takeAll(){
    auto list = d->children;
    d->children.clear();
    return list;
}

ServerManageModelItem::Type ServerManageModelItem::type(){
    return d->type;
}

QString ServerManageModelItem::name() const{
    return d->name;
}

QString ServerManageModelItem::path() const{
    return d->path;
}

bool ServerManageModelItem::expanded(){
    return d->expanded;
}

void ServerManageModelItem::setExpanded(bool expanded){
    d->expanded = expanded;
}

long long ServerManageModelItem::sid(){
    if(d->type==Server){
        return d->id;
    }else if(d->type==Folder || d->type==File){
        return d->parent->sid();
    }
    return 0;
}

long long ServerManageModelItem::pid(){
    if(d->type==Project){
        return d->id;
    }else if(d->type==Server){
        return d->parent->pid();
    }else if(d->type==Folder || d->type==File){
        return d->parent->pid();
    }
    return 0;
}

bool ServerManageModelItem::isLoading(){
    return d->loading;
}
void ServerManageModelItem::setLoading(bool state){
    d->loading = state;
}

ServerManageModelItem* ServerManageModelItem::findChild(long long id,bool project){
    if(project){
        for(auto one:d->children){
            if(one->type()==ServerManageModelItem::Project && one->pid()){
                return one;
            }
        }
    }else{
        for(auto one:d->children){
            if(one->type()==ServerManageModelItem::Server && one->sid()){
                return one;
            }else if(one->type()==ServerManageModelItem::Project){
                return one->findChild(id,false);
            }
        }
    }
    return nullptr;
}

ServerManageModelItem* ServerManageModelItem::findChild(const QString& path){
    foreach(auto one,d->children){
        const QString oPath = one->path();
        if(oPath==path){
            return one;
        }else if(path.startsWith(oPath)){
            Type type = one->type();
            if(type==ServerManageModelItem::Folder || type==ServerManageModelItem::Server){
                auto ret = one->findChild(path);
                if(ret!=nullptr){
                    return ret;
                }
            }
        }
    }
    return nullptr;
}




class ServerManageModelPrivate{
public:
    ServerManageModelPrivate():projectIcon(QString::fromUtf8(":/Resource/icons/DocumentsFolder_16x.svg")),serverIcon(":/Resource/icons/RemoteServer_16x.svg"){}
    ServerManageModelItem* root = nullptr;
    QIcon projectIcon;
    QIcon serverIcon;
    QMap<QString,QString> addons;
    QFileIconProvider* provider;
    FileItemSorting* sorting;
};


ServerManageModel::ServerManageModel(QObject *parent)
    : QAbstractItemModel{parent}
{
    d = new ServerManageModelPrivate;
    d->root = new ServerManageModelItem();
    d->provider = new QFileIconProvider();
    d->sorting = new FileItemSorting(ServerManageModel::Name,true);

    auto model = ResourceManagerModel::getInstance();

    if(model!=nullptr){
        auto root = model->rootItem();
        int count = root->childrenCount();
        for(int i=0;i<count;i++){
            auto one = root->childAt(i);
            this->openProject(one->pid(),one->title());
        }
    }
}

ServerManageModel::~ServerManageModel(){
    delete d->provider;
    delete d->root;
    delete d;
}

// Header:
QVariant ServerManageModel::headerData(int section, Qt::Orientation orientation, int role ) const {
    if(role==Qt::DisplayRole){
        if(section==Name){
            return tr("Name");
        }
    }
    return {};
}
// Basic functionality:
QModelIndex ServerManageModel::index(int row, int column,const QModelIndex &parent ) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ServerManageModelItem *parentItem;

    if (!parent.isValid())
        parentItem = d->root;
    else
        parentItem = static_cast<ServerManageModelItem*>(parent.internalPointer());

    ServerManageModelItem *childItem = parentItem->childAt(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex ServerManageModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    ServerManageModelItem *childItem = static_cast<ServerManageModelItem*>(index.internalPointer());
    if(childItem==d->root || childItem==nullptr){
        return QModelIndex();
    }

    ServerManageModelItem *parentItem = childItem->parent();
    if (parentItem == d->root || parentItem==nullptr)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

int ServerManageModel::rowCount(const QModelIndex &parent) const {
    if (!parent.isValid())
        return d->root->childrenCount();

    ServerManageModelItem* parentItem = static_cast<ServerManageModelItem*>(parent.internalPointer());
    if(parentItem!=nullptr){
        return parentItem->childrenCount();
    }else{
        return 0;
    }
}

int ServerManageModel::columnCount(const QModelIndex &parent) const{
    return Max;
}

QVariant ServerManageModel::data(const QModelIndex &index, int role) const{
    if (!index.isValid())
        return QVariant();
    if(role == Qt::DisplayRole){
        ServerManageModelItem* item = static_cast<ServerManageModelItem*>(index.internalPointer());
        int col = index.column();
        ServerManageModelItem::Type type = item->type();
        if(col==Name){
            if(item->isLoading()){
                return QString("%1(%2)").arg(item->name(),tr("Loading"));
            }else{
                return item->name();
            }
        }
    }else if(role == Qt::DecorationRole){
        if(index.column() == Name){
            ServerManageModelItem* item = static_cast<ServerManageModelItem*>(index.internalPointer());
            //DownloadFolder
            ServerManageModelItem::Type type = item->type();
            if(type==ServerManageModelItem::Project){
                return d->projectIcon;
            }else if(type==ServerManageModelItem::Server){
                return d->serverIcon;
            }else if(type==ServerManageModelItem::Folder){
                return d->provider->icon(QFileIconProvider::Folder);
            }else if(type==ServerManageModelItem::File){
                //qDebug()<<"path:"<<item->path();
                //return d->provider->icon(QFileInfo(item->name()));
                return d->provider->icon(QFileIconProvider::File);
            }
        }else{
            return QVariant();
        }
    }else if(role==Qt::EditRole){
         if(index.column()==Name){
            ServerManageModelItem* item = static_cast<ServerManageModelItem*>(index.internalPointer());
            return item->name();
        }
    }
    return QVariant();
}

bool ServerManageModel::hasChildren(const QModelIndex &parent)const{
    if(parent.isValid()){
        auto item = static_cast<ServerManageModelItem*>(parent.internalPointer());
        if(item!=nullptr){
            switch(item->type()){
            case ServerManageModelItem::Solution:
            case ServerManageModelItem::Project:
            case ServerManageModelItem::Server:
                return true;
            case ServerManageModelItem::File:
                return false;
            case ServerManageModelItem::Folder:
            {
                if(item->childrenCount()>0){
                    return true;
                }else{

                    return !item->expanded();
                }
            }
            }
        }
    }
    return true;
}

bool ServerManageModel::setData(const QModelIndex &index, const QVariant &value, int role){
    if(role==Qt::EditRole){
        if(index.column() ==Name ){
            QString name = value.toString();
            ServerManageModelItem* item = static_cast<ServerManageModelItem*>(index.internalPointer());
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

Qt::ItemFlags ServerManageModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    flags = flags | Qt::ItemIsDragEnabled ;
    if(index.column()==Name){
        ServerManageModelItem* item = static_cast<ServerManageModelItem*>(index.internalPointer());
        ServerManageModelItem::Type type = item->type();
        if(type==ServerManageModelItem::File || type==ServerManageModelItem::Folder){
            flags |= Qt::ItemIsEditable;
        }
    }
    return flags;
}

void ServerManageModel::appendItem(ServerManageModelItem* parent,ServerManageModelItem* item){
    int position = parent->childrenCount();
    QModelIndex parentIndex;
    if(parent!=d->root){
        parentIndex = createIndex(parent->row(),0,parent);
    }
    beginInsertRows(parentIndex,position,position);
    parent->appendItem(item);
    endInsertRows();
}

void ServerManageModel::appendItems(QList<FileItem> list,ServerManageModelItem* parent){
    int position = parent->childrenCount();
    QModelIndex parentIndex;
    if(parent!=d->root){
        parentIndex = createIndex(parent->row(),0,parent);
    }
    int size = list.size();
    beginInsertRows(parentIndex,position,position + size);
    qSort(list.begin(),list.end(),*d->sorting);
    for(auto one:list){
        //auto item = new ServerManageModelItem(one.)
        auto item = new ServerManageModelItem(one,parent);
        parent->appendItem(item);
    }
    endInsertRows();
}

void ServerManageModel::refreshItems(QList<FileItem> list,ServerManageModelItem* parent){
    QModelIndex index;
    if(parent!=d->root){
        index = createIndex(parent->row(),0,parent);
    }
    int childrenCount = parent->childrenCount();
    if(childrenCount>0){
        beginRemoveRows(index,0,childrenCount - 1);
        auto rows = parent->takeAll();
        endRemoveRows();
        qDeleteAll(rows);
    }
    this->appendItems(list,parent);
}

void ServerManageModel::openProject(long long id,const QString name){
    auto item = new ServerManageModelItem(ServerManageModelItem::Project,id,name,d->root);
    //find all site
    auto instance = NetworkManager::getInstance();
    if(id>0){
        SiteStorage db;
        QList<SiteRecord> list = db.list(id);
        for(auto one:list){
            auto server = new ServerManageModelItem(ServerManageModelItem::Server,one.id,one.name,one.path,item);
            item->appendItem(server);
            //init request
            if(instance!=nullptr){
                instance->initRequest(one.id,one.type);
            }
        }
    }
    this->appendItem(d->root,item);
}

ServerManageModelItem* ServerManageModel::find(const QString& path){
    return d->root->findChild(path);
}

ServerManageModelItem* ServerManageModel::find(long long id,bool project){
    return d->root->findChild(id,project);
}

ServerManageModelItem* ServerManageModel::find(long long id,const QString& path){
    auto server = d->root->findChild(id,false);
    if(server!=nullptr){
        return server->findChild(path);
    }else{
        return nullptr;
    }
}

ServerManageModelItem* ServerManageModel::rootItem(){
    return d->root;
}

}