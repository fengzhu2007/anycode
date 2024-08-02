#include "file_transfer_model.h"
#include "panes/resource_manager/resource_manager_model.h"
#include "panes/resource_manager/resource_manager_model_item.h"
#include "storage/site_storage.h"
#include <memory.h>
#include <QFileInfo>
#include <QIcon>
namespace ady{

class FileTransferModelItemPrivate{
public:
    FileTransferModelItem::Type type;
    FileTransferModelItem::Mode mode;
    long long id;
    long long filesize=-1l;
    float status=-1;
    FileTransferModelItem* parent=nullptr;
    FileTransferJob* data = nullptr;
    QString name;
    QString src;
    QString dest;
    QList<FileTransferModelItem*> children;
    //QList<std::shared_ptr<FileTransferModelItem>>children;
};

FileTransferModelItem::FileTransferModelItem(){
    d = new FileTransferModelItemPrivate;
    d->id = -1;
    d->type = Solution;
    d->mode = None;
    d->filesize = -1l;
}

FileTransferModelItem::FileTransferModelItem(Type type,long long id,const QString& name,FileTransferModelItem* parent){
    d = new FileTransferModelItemPrivate;
    d->mode = None;
    d->type = type;
    d->id = id;
    d->name = name;
    d->parent = parent;
    d->filesize = -1l;

}

FileTransferModelItem::FileTransferModelItem(Mode mode,const QString& src,const QString& dest,FileTransferModelItem* parent){
    d = new FileTransferModelItemPrivate;
    d->type = Job;
    d->mode = mode;
    d->src = src;
    d->dest = dest;
    d->parent = parent;
    QFileInfo fi(src);
    d->name = fi.fileName();
    d->filesize = fi.size();
}

FileTransferModelItem::~FileTransferModelItem(){

    delete d;
}

int FileTransferModelItem::childrenCount(){
    return d->children.size();
}

int FileTransferModelItem::row(){
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

void FileTransferModelItem::appendItems(QList<FileTransferModelItem*>items){

}

void FileTransferModelItem::appendItem(FileTransferModelItem* item){
    d->children.push_back(item);
}

FileTransferModelItem* FileTransferModelItem::parent(){
    return d->parent;
}

FileTransferModelItem* FileTransferModelItem::childAt(int i){
    return d->children.at(i);
}

FileTransferJob* FileTransferModelItem::data(){
    return d->data;
}

FileTransferModelItem::Type FileTransferModelItem::type(){
    return d->type;
}

FileTransferModelItem::Mode FileTransferModelItem::mode(){
    return d->mode;
}

QString FileTransferModelItem::name() const{
    return d->name;
}

QString FileTransferModelItem::source() const{
    return d->src;
}

QString FileTransferModelItem::destination() const{
    return d->dest;
}

unsigned long long FileTransferModelItem::filesize(){
    return d->filesize;
}







class FileTransferModelPrivate{
public:
    FileTransferModelPrivate():projectIcon(QString::fromUtf8(":/Resource/icons/DocumentsFolder_16x.svg")),serverIcon(":/Resource/icons/RemoteServer_16x.svg"){}
    FileTransferModelItem* root = nullptr;
    QIcon projectIcon;
    QIcon serverIcon;
    QIcon downloadFolderIcon;
    QIcon uploadFolderIcon;
    QIcon downloadFileIcon;
    QIcon uploadFileIcon;
};

FileTransferModel::FileTransferModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    d = new FileTransferModelPrivate;
    d->root = new FileTransferModelItem;

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

FileTransferModel::~FileTransferModel(){
    delete d->root;
    delete d;
}

QVariant FileTransferModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role==Qt::DisplayRole){
        if(section==Name){
            return tr("Name");
        }else if(section==FileSize){
            return tr("File Size");
        }else if(section==Src){
            return tr("Source");
        } if(section==Dest){
            return tr("Destination");
        }else if(section==Status){
            return tr("Status");
        }
    }
    return {};
}

QModelIndex FileTransferModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    FileTransferModelItem *parentItem;

    if (!parent.isValid())
        parentItem = d->root;
    else
        parentItem = static_cast<FileTransferModelItem*>(parent.internalPointer());

    FileTransferModelItem *childItem = parentItem->childAt(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex FileTransferModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    FileTransferModelItem *childItem = static_cast<FileTransferModelItem*>(index.internalPointer());
    if(childItem==d->root || childItem==nullptr){
        return QModelIndex();
    }

    FileTransferModelItem *parentItem = childItem->parent();
    if (parentItem == d->root || parentItem==nullptr)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

int FileTransferModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return d->root->childrenCount();

    FileTransferModelItem* parentItem = static_cast<FileTransferModelItem*>(parent.internalPointer());
    if(parentItem!=nullptr){
        return parentItem->childrenCount();
    }else{
        return 0;
    }
}

int FileTransferModel::columnCount(const QModelIndex &parent) const
{
    return Max;
}

QVariant FileTransferModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if(role == Qt::DisplayRole){
        FileTransferModelItem* item = static_cast<FileTransferModelItem*>(index.internalPointer());
        int col = index.column();
        FileTransferModelItem::Type type = item->type();
        if(col==Name){
            return item->name();
        }else if(type==FileTransferModelItem::Job && col==FileSize){
            return item->filesize();
        }else if(type==FileTransferModelItem::Job && col==Src){
            return item->source();
        }else if(type==FileTransferModelItem::Job && col==Dest){
            return item->destination();
        }else if(type==FileTransferModelItem::Job && col==Status){

        }
    }else if(role == Qt::DecorationRole){
        if(index.column() == Name){
            FileTransferModelItem* item = static_cast<FileTransferModelItem*>(index.internalPointer());
            //DownloadFolder
            FileTransferModelItem::Type type = item->type();
            if(type==FileTransferModelItem::Project){
                return d->projectIcon;
            }else if(type==FileTransferModelItem::Server){
                return d->serverIcon;
            }
        }else{
            return QVariant();
        }
    }
    return QVariant();
}

void FileTransferModel::appendItem(FileTransferModelItem* parent,FileTransferModelItem* item){
    int position = parent->childrenCount();
    QModelIndex parentIndex;
    if(parent!=d->root){
       parentIndex = createIndex(parent->row(),0,parent);
    }
    //qDebug()<<"parent:"<<parent<<"root:"<<d->root<<"pos"<<position<<parentIndex;
    beginInsertRows(parentIndex,position,position);
    parent->appendItem(item);
    endInsertRows();
}

void FileTransferModel::openProject(long long id,const QString name){
    auto item = new FileTransferModelItem(FileTransferModelItem::Project,id,name,d->root);
    //find all site
    if(id>0){
        SiteStorage db;
        QList<SiteRecord> list = db.list(id);
        for(auto one:list){
            auto server = new FileTransferModelItem(FileTransferModelItem::Server,one.id,one.name,item);
            item->appendItem(server);
        }
    }
    this->appendItem(d->root,item);
}

}
