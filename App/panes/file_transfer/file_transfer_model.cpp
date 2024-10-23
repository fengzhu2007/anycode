#include "file_transfer_model.h"
#include "panes/resource_manager/resource_manager_model.h"
#include "panes/resource_manager/resource_manager_model_item.h"
#include "storage/site_storage.h"
#include "job_thread.h"
#include "network/network_manager.h"
#include "common/utils.h"
//#include <memory.h>
#include <QFileInfo>
#include <QIcon>
#include <QMutex>
#include <QWaitCondition>
#include <QFileIconProvider>
namespace ady{




class FileTransferModelItemPrivate{
public:
    FileTransferModelItem::Type type;
    FileTransferModelItem::Mode mode;
    FileTransferModelItem::State state;
    long long id;
    long long filesize=-1l;
    float status=-1;
    FileTransferModelItem* parent=nullptr;
    FileTransferJob* data = nullptr;
    QString name;
    QString src;//local for download and upload
    QString dest;//remote for download and upload

    QList<FileTransferModelItem*> children;
    //QList<std::shared_ptr<FileTransferModelItem>>children;
};

int FileTransferModelItem::seq = 1;


FileTransferModelItem::FileTransferModelItem(){
    d = new FileTransferModelItemPrivate;
    d->id = -1;
    d->type = Solution;
    d->mode = None;
    d->state = Any;
    d->filesize = -1l;
}

FileTransferModelItem::FileTransferModelItem(Type type,long long id,const QString& name,const QString& path,FileTransferModelItem* parent){
    d = new FileTransferModelItemPrivate;
    d->mode = None;
    d->type = type;
    d->state = Any;
    d->id = id;
    d->name = name;
    d->src = path;//project local path   or  site remote default path
    d->parent = parent;
    d->filesize = -1l;

}

FileTransferModelItem::FileTransferModelItem(Mode mode,bool is_file,const QString& src,const QString& dest,FileTransferModelItem* parent){
    d = new FileTransferModelItemPrivate;
    d->type = is_file ? Job:JobGroup;
    d->mode = mode;
    d->state = Wait;
    d->id = FileTransferModelItem::seq++;//
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
    for(auto one:items){
        d->children << one;
    }
}

void FileTransferModelItem::insertItems(int position,QList<FileTransferModelItem*> items){
    for(auto one:items){
        d->children.insert(position++,one);
    }
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

FileTransferModelItem* FileTransferModelItem::findByProjectId(long long id){
    for(auto one:d->children){
        if(one->type()==Project && one->id()){
            return one;
        }
    }
    return nullptr;
}
FileTransferModelItem* FileTransferModelItem::findBySiteId(long long id){
    for(auto one:d->children){
        if(one->type()==Server && one->id()){
            return one;
        }
    }
    return nullptr;
}

FileTransferModelItem* FileTransferModelItem::first(State state){
    for(auto one:d->children){
        if(state == one->state() || state==FileTransferModelItem::Any){
            return one;
        }
    }
    return nullptr;
}

FileTransferModelItem* FileTransferModelItem::take(int position){
    return d->children.takeAt(position);
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

long long FileTransferModelItem::id(){
    return d->id;
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


FileTransferModelItem::State FileTransferModelItem::state(){
    return d->state;
}

void FileTransferModelItem::setState(State state){
    d->state = state;
}
unsigned long long FileTransferModelItem::filesize(){
    return d->filesize;
}





FileTransferModel* FileTransferModel::instance = nullptr;

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
    QMutex mutex;
    QWaitCondition cond;
    QList<JobThread*> threads;
    QFileIconProvider* provider;
};


FileTransferModel* FileTransferModel::getInstance(){
    if(instance==nullptr){
        instance = new FileTransferModel;
    }
    return instance;
}

void FileTransferModel::destory(){
    if(instance!=nullptr){
        delete instance;
        instance = nullptr;
    }
}

FileTransferModel::FileTransferModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    d = new FileTransferModelPrivate;
    d->root = new FileTransferModelItem;
    d->provider = new QFileIconProvider();

    auto model = ResourceManagerModel::getInstance();

    if(model!=nullptr){
        auto root = model->rootItem();
        int count = root->childrenCount();
        for(int i=0;i<count;i++){
            auto one = root->childAt(i);
            this->openProject(one->pid(),one->title(),one->path());
        }
    }
}

FileTransferModel::~FileTransferModel(){
    delete d->provider;
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
            return Utils::readableFilesize(item->filesize());
        }else if((type==FileTransferModelItem::Job || type==FileTransferModelItem::JobGroup) && col==Src){
            return item->source();
        }else if((type==FileTransferModelItem::Job || type==FileTransferModelItem::JobGroup) && col==Dest){
            return item->destination();
        }else if((type==FileTransferModelItem::Job || type==FileTransferModelItem::JobGroup) && col==Status){

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
            }else if(type==FileTransferModelItem::Job){
                return d->provider->icon(QFileIconProvider::File);
            }else if(type==FileTransferModelItem::JobGroup){
                return d->provider->icon(QFileIconProvider::Folder);
            }
        }else{
            return QVariant();
        }
    }
    return QVariant();
}

void FileTransferModel::appendItem(FileTransferModelItem* parent,FileTransferModelItem* item){
    QMutexLocker locker(&d->mutex);
    int position = parent->childrenCount();
    QModelIndex parentIndex;
    if(parent!=d->root){
       parentIndex = createIndex(parent->row(),0,parent);
    }
    beginInsertRows(parentIndex,position,position);
    parent->appendItem(item);
    endInsertRows();
}

void FileTransferModel::insertFrontItems(FileTransferModelItem* parent,QList<FileTransferModelItem*> items,FileTransferModelItem::State state){
    if(items.size()<=0){
       return ;
    }
    QMutexLocker locker(&d->mutex);
    int total = parent->childrenCount();
    int position = -1;
    if(state == FileTransferModelItem::Wait){
       position = total;
       for(int i=0;i<total;i++){
            auto one = parent->childAt(i);
            if(one->state()== FileTransferModelItem::Wait){
                position = i;
                break;
            }
       }
    }else if(state == FileTransferModelItem::Doing){
       position = total;
       for(int i=0;i<total;i++){
            auto one = parent->childAt(i);
            FileTransferModelItem::State state = one->state();
            if(state== FileTransferModelItem::Doing || state== FileTransferModelItem::Wait){
                position = i;
                break;
            }
       }
    }else if(state == FileTransferModelItem::Failed){
       position = 0;
    }
    if(position>-1){
        auto index = createIndex(parent->row(),0,parent);//site modelindex (job parent modelindex)
        beginInsertRows(index,position,position + items.size() - 1);
        parent->insertItems(position,items);
        endInsertRows();
    }
}

void FileTransferModel::removeItem(FileTransferModelItem* item){
    QMutexLocker locker(&d->mutex);
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

void FileTransferModel::removeItem(long long siteid,long long id){
    QMutexLocker locker(&d->mutex);
    int pTotal = d->root->childrenCount();
    for(int i=0;i<pTotal;i++){
        auto proj = d->root->childAt(i);
        int sTotal = proj->childrenCount();
        for(int j=0;j<sTotal;j++){
            auto site = proj->childAt(j);
            if(site->id() == siteid){
                int jTotal = site->childrenCount();
                for(int k=0;k<jTotal;k++){
                    auto one = site->childAt(k);
                    if(one->id()==id){

                        QModelIndex parentIndex = createIndex(site->row(),0,site);
                        int position = one->row();
                        beginRemoveRows(parentIndex,position,position);
                        auto child = site->take(position);
                        endRemoveRows();
                        //delete child;
                        delete child;

                        return ;
                    }
                }
                return ;
            }
        }
    }
}

void FileTransferModel::openProject(long long id,const QString& name,const QString& path){
    auto item = new FileTransferModelItem(FileTransferModelItem::Project,id,name,path,d->root);
    //find all site
    qDebug()<<"openProject"<<path<<item;
    if(id>0){
        SiteStorage db;
        QList<SiteRecord> list = db.list(id,1);
        for(auto one:list){
            auto server = new FileTransferModelItem(FileTransferModelItem::Server,one.id,one.name,one.path,item);
            item->appendItem(server);
        }
    }
    this->appendItem(d->root,item);
}

void FileTransferModel::addJob(UploadData* data){
    auto proj = d->root->findByProjectId(data->pid);
    qDebug()<<"addJob"<<data->pid<<data->source<<proj;//pid is error
    if(proj!=nullptr){
        auto site = proj->findBySiteId(data->siteid);
        if(site!=nullptr){
            //add file or folder
            QMutexLocker locker(&d->mutex);
            auto index = createIndex(site->row(),0,site);//site modelindex (job parent modelindex)
            int position = site->childrenCount();

            QString source = data->source;//absolute path
            QString destination = data->dest;//absolute path
            //qDebug()<<"addJob"<<source;
            if(destination.isEmpty()){
                auto req = NetworkManager::getInstance()->request(data->siteid);
                if(req!=nullptr){
                    const QString projectPath = proj->source() + "/";
                    const QString remotePath = site->source();
                    if(source.startsWith(projectPath)){
                        QString rSource = source.mid(projectPath.length());
                        //qDebug()<<source<<projectPath<<"rSource:"<<rSource;
                        QString rDest = req->matchToPath(rSource,true);
                        if(rDest.isEmpty()){
                            return ;
                        }
                        destination = remotePath + rDest;
                    }else{
                        return ;//not match project
                    }
                }else{
                    return;
                }
            }
            beginInsertRows(index,position,position);
            site->appendItem(new FileTransferModelItem(FileTransferModelItem::Upload,data->is_file,source,destination,site));
            endInsertRows();
        }
    }
}

void FileTransferModel::start(int num){
    QMutexLocker locker(&d->mutex);

}



FileTransferModelItem* FileTransferModel::take(int siteid){
    QMutexLocker locker(&d->mutex);
    if( d->root->childrenCount()==0 ){
        d->cond.wait(&d->mutex,5*1000);
    }
    FileTransferModelItem *item = nullptr;
    while((item=this->get(siteid))==nullptr){
        d->cond.wait(&d->mutex,5*1000);
    }
    return item;
}

FileTransferModelItem* FileTransferModel::get(int siteid){
    int pTotal = d->root->childrenCount();
begin:
    for(int i=0;i<pTotal;i++){
        auto proj = d->root->childAt(i);
        int sTotal = proj->childrenCount();
        for(int j=0;j<sTotal;j++){
            auto site = proj->childAt(j);
            if(siteid==0 || site->id() == siteid){
                auto item = site->first(FileTransferModelItem::Wait);
                if(item!=nullptr){
                    return item;
                }else if(siteid>0){
                    siteid = 0;
                    goto begin;
                }
            }
        }
    }
    return nullptr;
}

}
