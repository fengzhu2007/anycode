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
#include <QDebug>
namespace ady{




class FileTransferModelItemPrivate{
public:
    FileTransferModelItem::Type type;
    FileTransferModelItem::Mode mode;
    FileTransferModelItem::State state;
    long long id;
    long long filesize=-1l;
    float progress = 0;
    bool matched;
    FileTransferModelItem* parent=nullptr;
    QString name;
    QString src;//local for download and upload
    QString dest;//remote for download and upload
    QString errorMsg;

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
    d->matched = false;
    //qDebug()<<"item init:"<<this<<"solution";
}

FileTransferModelItem::FileTransferModelItem(Type type,long long id,const QString& name,const QString& path,FileTransferModelItem* parent){
    d = new FileTransferModelItemPrivate;
    d->mode = None;
    d->type = type;
    d->state = Any;
    d->id = id;
    d->name = name;
    if(type==Project){
        d->src = path;//project local path
    }else if(type==Server){
        d->src = parent->source();//project path
        d->dest = path;//site remote default path
    }
    d->parent = parent;
    d->filesize = -1l;
    d->matched = false;
    //qDebug()<<"item init:"<<this<<"project or site"<<name;

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
    d->matched = true;
    // qDebug()<<"item init:"<<this<<"file"<<src;
}

FileTransferModelItem::~FileTransferModelItem(){
    // qDebug()<<"item destory"<<this<<d->src;
    qDeleteAll(d->children);
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
        if(one->type()==Project && one->id() == id){
            return one;
        }
    }
    return nullptr;
}
FileTransferModelItem* FileTransferModelItem::findBySiteId(long long id){
    for(auto one:d->children){
        if(one->type()==Server && one->id() == id){
            return one;
        }else if(one->type()==Project){
            auto item = one->findBySiteId(id);
            if(item!=nullptr){
                return item;
            }
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

FileTransferModelItem* FileTransferModelItem::take(int i){
    return d->children.takeAt(i);
}

QList<FileTransferModelItem*> FileTransferModelItem::take(int from,int end){
    QList<FileTransferModelItem*> list;
    auto iter = d->children.begin();
    int i = 0;
    while(iter!=d->children.end()){
        if(i>=from){
            if(i<=end){
                list<<(*iter);
                d->children.erase(iter);
            }else{
                break;
            }
        }
        i++;
        iter++;
    }
    return list;
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

float FileTransferModelItem::progress(){
    return d->progress;
}

void FileTransferModelItem::setProgress(float progress){
    d->progress = progress;
}

QString FileTransferModelItem::errorMsg(){
    return d->errorMsg;
}

void FileTransferModelItem::setErrorMsg(const QString& errormsg){
    d->errorMsg = errormsg;
}


bool FileTransferModelItem::matchedPath(){
    return d->matched;
}


void FileTransferModelItem::setMatchedPath(bool matched){
    d->matched = matched;
}

void FileTransferModelItem::setFilesize(long long filesize){
    d->filesize = filesize;
}

long long FileTransferModelItem::filesize(){
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
    FileTransferModel::State state;
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
    qRegisterMetaType<QVector<int>>("QVector<int>");
    qRegisterMetaType<QVector<int>>("QList<FileItem>");

    connect(this,&FileTransferModel::notifyProgress,this,&FileTransferModel::onUpdateProgress);

}

FileTransferModel::~FileTransferModel(){
    this->finish();//finish all thread

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
    //qDebug()<<"item:"<<childItem<<index.row();

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
        auto mode = item->mode();
        if(col==Name){
            return item->name();
        }else if(type==FileTransferModelItem::Job && col==FileSize){
            return Utils::readableFilesize(item->filesize());
        }else if((type==FileTransferModelItem::Job || type==FileTransferModelItem::JobGroup) && col==Src){
            return mode==FileTransferModelItem::Upload?item->source():item->destination();
        }else if((type==FileTransferModelItem::Job || type==FileTransferModelItem::JobGroup) && col==Dest){
            //return item->destination();
            return mode!=FileTransferModelItem::Upload?item->source():item->destination();
        }else if((type==FileTransferModelItem::Job || type==FileTransferModelItem::JobGroup) && col==Status){
            FileTransferModelItem::State state = item->state();
            if(state==FileTransferModelItem::Doing){
                return QString::fromUtf8("%1").arg(qRound(item->progress() * 1000) / 10)+"%";
            }else if(state==FileTransferModelItem::Failed){
                return item->errorMsg();
            }
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



void FileTransferModel::insertFrontAndRemove(long long siteid,long long id,QFileInfoList list,FileTransferModelItem::State state){
    QMutexLocker locker(&d->mutex);
    auto site = d->root->findBySiteId(siteid);
    if(site!=nullptr){
       //remove id
       bool matched = false;
       QString matchedPath;
       int jTotal = site->childrenCount();
       for(int k=0;k<jTotal;k++){
            auto one = site->childAt(k);
            if(one->id()==id){
                matched = one->matchedPath();
                matchedPath = one->destination();
                QModelIndex parentIndex = createIndex(site->row(),0,site);
                int position = one->row();
                //qDebug()<<"remote position"<<position<<site->childrenCount();
                beginRemoveRows(parentIndex,position,position);
                auto child = site->take(position);
                endRemoveRows();
                //qDebug()<<"remote position"<<position<<site->childrenCount()<<child<<child->source();
                //delete child;
                delete child;
                break;
            }
       }
       if(matchedPath.isEmpty()){
            return ;
       }
       //add list
       int length = list.length();
       QList<FileTransferModelItem*> items;
       NetworkRequest* req = nullptr;
       if(matched && (req = NetworkManager::getInstance()->request(siteid))!=nullptr){
            const QString projectPath = site->source() + "/";
            const QString remotePath = site->destination();
            for(int i=0;i<length;i++){
                QFileInfo fi = list.at(i);
                const QString source = fi.absoluteFilePath();
                QString destination;
                if(source.startsWith(projectPath)){
                    QString rSource = source.mid(projectPath.length());
                    QString rDest = req->matchToPath(rSource,true);
                    if(rDest.isEmpty()){
                        qDebug()<<"upload igore path:"<<source;
                        continue;
                    }
                    destination = remotePath + rDest;
                }else{
                    destination = matchedPath + "/" + fi.fileName() ;
                }
                auto item = new FileTransferModelItem(FileTransferModelItem::Upload,fi.isFile(),source,destination,site);
                item->setMatchedPath(true);
                items<<item;
            }
       }else{
            QString remote = matchedPath;//parent item remote path;
            if(!remote.endsWith("/")){
                remote += "/";
            }
            for(int i=0;i<length;i++){
                QFileInfo fi = list.at(i);
                auto item = new FileTransferModelItem(FileTransferModelItem::Upload,fi.isFile(),fi.absoluteFilePath(),remote+fi.fileName(),site);
                item->setMatchedPath(false);
                items<<item;
            }
       }
       this->insertFrontItems(site,items,state);
    }
}
void FileTransferModel::insertFrontAndRemove(long long siteid,long long id,QList<FileItem> list,FileTransferModelItem::State state){
    QMutexLocker locker(&d->mutex);
    auto site = d->root->findBySiteId(siteid);
    if(site!=nullptr){
       //remove id
       bool matched = false;
       QString matchedPath;
       int jTotal = site->childrenCount();
       for(int k=0;k<jTotal;k++){
            auto one = site->childAt(k);
            if(one->id()==id){
                matched = one->matchedPath();
                matchedPath = one->source();
                QModelIndex parentIndex = createIndex(site->row(),0,site);
                int position = one->row();
                beginRemoveRows(parentIndex,position,position);
                auto child = site->take(position);
                endRemoveRows();
                //delete child;
                delete child;
                break;
            }
       }
       if(matchedPath.isEmpty()){
            return ;
       }
       //add list
       int length = list.length();
       QList<FileTransferModelItem*> items;
       NetworkRequest* req = nullptr;
       if(matched && (req = NetworkManager::getInstance()->request(siteid))!=nullptr){
            const QString localPath = site->source() + "/";//local project path
            const QString remotePath = site->destination();//remote path
            for(int i=0;i<length;i++){
                auto one = list.at(i);
                QString source;
                const QString destination = one.path;//remote absolute path
                if(destination.startsWith(remotePath)){
                    QString rDest = destination.mid(remotePath.length());
                    QString rSource = req->matchToPath(rDest,false);
                    if(rSource.isEmpty()){
                        qDebug()<<"download igore path:"<<destination;
                        continue;
                    }
                    source = localPath + rSource;
                }else{
                    source = matchedPath + "/" + one.name ;
                }
                auto item = new FileTransferModelItem(FileTransferModelItem::Download,one.type==FileItem::File,source,one.path,site);
                item->setMatchedPath(true);
                items<<item;
            }
       }else{
            QString local = matchedPath;//parent item local path
            if(!local.endsWith("/")){
                local += "/";
            }
            for(auto one:list){
                auto item = new FileTransferModelItem(FileTransferModelItem::Download,one.type==FileItem::File,local+one.name,one.path,site);
                item->setMatchedPath(false);
                items<<item;
            }
       }
       this->insertFrontItems(site,items,state);
    }
}

void FileTransferModel::insertFrontItems(FileTransferModelItem* parent,QList<FileTransferModelItem*> items,FileTransferModelItem::State state){
    if(items.size()<=0){
       return ;
    }
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
       d->cond.wakeAll();
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

void FileTransferModel::removeProject(long long id){
    if(id==0){
        return ;
    }
    int pTotal = d->root->childrenCount();
    for(int i=0;i<pTotal;i++){
         auto proj = d->root->childAt(i);
        if(proj->id() == id){
            return this->removeItem(proj);
         }
    }
}


void FileTransferModel::removeAllItems(FileTransferModelItem::State state){
    QMutexLocker(&d->mutex);
    int pTotal = d->root->childrenCount();
    for(int i=0;i<pTotal;i++){
         auto proj = d->root->childAt(i);
         int sTotal = proj->childrenCount();
         for(int j=0;j<sTotal;j++){
            auto site = proj->childAt(j);
            int total = site->childrenCount();
            if(total==0){
                continue;
            }
            int from = -1;
            int end = -1;
            if(state == FileTransferModelItem::Any){
                from = 0;
                end = total - 1;
            }else{
                for(int k=0;k<total;k++){
                    auto item = site->childAt(k);
                    if(item->state()==state){
                        if(from==-1){
                            from = k;//find first delete position
                        }
                    }else if(from>-1 && end==-1 && item->state()!=state){
                        end = k;
                        break;
                    }
                }
                if(from>-1 && end==-1){
                    end = total - 1;
                }
            }
            if(from>-1 && end>-1){
                this->remoteItems(site,from,end);
            }
         }
    }
}


void FileTransferModel::removeItems(QModelIndexList list,FileTransferModelItem::State state){
    QMutexLocker(&d->mutex);
    FileTransferModelItem* site = nullptr;
    for(auto one:list){
         auto item = static_cast<FileTransferModelItem*>(one.internalPointer());
         auto type = item->type();
         if((type==FileTransferModelItem::Job || type==FileTransferModelItem::JobGroup) && (state==FileTransferModelItem::Any || item->state() == state)){
            site = item->parent();

            auto index = createIndex(site->row(),0,site);
            //remove failed
            int from = item->row();
            beginRemoveRows(index,from,from);
            auto r = site->take(from);
            endRemoveRows();
            delete r;
         }
    }
}


void FileTransferModel::retryItems(QModelIndexList list){
    QMutexLocker(&d->mutex);
    FileTransferModelItem* site = nullptr;
    for(auto one:list){
        auto item = static_cast<FileTransferModelItem*>(one.internalPointer());
        auto type = item->type();
        if((type==FileTransferModelItem::Job || type==FileTransferModelItem::JobGroup) && item->state() == FileTransferModelItem::Failed){
            site = item->parent();

            auto index = createIndex(site->row(),0,site);
            //remove failed
            int from = item->row();
            beginRemoveRows(index,from,from);
            auto r = site->take(from);
            endRemoveRows();

            //add to wait
            int pos = site->childrenCount();
            beginInsertRows(index,pos,pos);
            r->setState(FileTransferModelItem::Wait);
            site->appendItem(r);
            endInsertRows();
         }
    }
}

void FileTransferModel::retryAllItems(){
    QMutexLocker(&d->mutex);
    int pTotal = d->root->childrenCount();
    for(int i=0;i<pTotal;i++){
         auto proj = d->root->childAt(i);
         int sTotal = proj->childrenCount();
         for(int j=0;j<sTotal;j++){
            auto site = proj->childAt(j);
            int from = -1;
            int end = -1;
            int total = site->childrenCount();
            for(int k=0;k<total;k++){
                auto item = site->childAt(k);
                if(item->state()==FileTransferModelItem::Failed){
                    if(from==-1){
                        from = k;
                    }
                    end = k;
                }else{
                    break;
                }
            }
            if(from>-1 && end>-1){
                auto index = createIndex(site->row(),0,site);
                //remove failed
                beginRemoveRows(index,from,end);
                auto list = site->take(from,end);
                endRemoveRows();

                //add to wait
                int pos = site->childrenCount();
                beginInsertRows(index,pos,pos+list.size() - 1);
                for(auto one:list){
                    one->setState(FileTransferModelItem::Wait);
                    site->appendItem(one);
                }
                endInsertRows();
            }
         }
    }
}

void FileTransferModel::remoteItems(FileTransferModelItem* site,int from,int end){
    auto index = createIndex(site->row(),0,site);
    beginRemoveRows(index,from,end);
    auto list = site->take(from,end);
    endRemoveRows();
    //delete list
    qDeleteAll(list);
}




void FileTransferModel::openProject(long long id,const QString& name,const QString& path){
    auto item = new FileTransferModelItem(FileTransferModelItem::Project,id,name,path,d->root);
    //find all site
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
    auto json = data->toJson();
    this->addUploadJob(json);
    /*QMutexLocker locker(&d->mutex);
    auto proj = d->root->findByProjectId(data->pid);
    if(proj!=nullptr){
        auto site = proj->findBySiteId(data->siteid);
        if(site!=nullptr){
            //add file or folder
            auto index = createIndex(site->row(),0,site);//site modelindex (job parent modelindex)
            int position = site->childrenCount();
            QString source = data->source;//absolute path
            QString destination = data->dest;//absolute path

            if(destination.isEmpty()){
                auto req = NetworkManager::getInstance()->request(data->siteid);
                if(req!=nullptr){
                    const QString projectPath = proj->source() + "/";
                    //qDebug()<<"addJob"<<source<<projectPath;
                    const QString remotePath = site->destination();
                    if(source.startsWith(projectPath)){
                        QString rSource = source.mid(projectPath.length());
                        QString rDest = req->matchToPath(rSource,true);
                        if(rDest.isEmpty()){
                            return ;
                        }
                        destination = remotePath + rDest;
                    }else if(source==proj->source()){
                        //upload project folder
                        destination = remotePath;
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
    d->cond.wakeAll();*/
}

void FileTransferModel::addUploadJob(QJsonObject data){
    QMutexLocker locker(&d->mutex);
    long long pid = data.find("pid")->toInt(0);
    long long siteid = data.find("siteid")->toInt();
    if(pid!=0 && siteid!=0){
        auto proj = d->root->findByProjectId(pid);
        if(proj!=nullptr){
            auto site = proj->findBySiteId(siteid);

            auto index = createIndex(site->row(),0,site);//site modelindex (job parent modelindex)
            int position = site->childrenCount();
            QString source = data.find("source")->toString();//absolute path
            QString destination = data.find("dest")->toString();//absolute path
            bool is_file = data.find("is_file")->toBool(true);
            bool matched = false;
            if(destination.isEmpty()){
                auto req = NetworkManager::getInstance()->request(siteid);
                if(req!=nullptr){
                    const QString projectPath = proj->source() + "/";
                    const QString remotePath = site->destination();
                    if(source.startsWith(projectPath)){
                        QString rSource = source.mid(projectPath.length());
                        QString rDest = req->matchToPath(rSource,true);
                        if(rDest.isEmpty()){
                            return ;
                        }
                        destination = remotePath + rDest;
                    }else if(source==proj->source()){
                        //upload project folder
                        destination = remotePath;
                    }else{
                        return ;//not match project
                    }
                }else{
                    return;
                }
                matched = true;
            }
            beginInsertRows(index,position,position);
            auto item = new FileTransferModelItem(FileTransferModelItem::Upload,is_file,source,destination,site);
            item->setMatchedPath(matched);
            site->appendItem(item);
            endInsertRows();

        }
    }
    d->cond.wakeAll();
}
void FileTransferModel::addDownloadJob(QJsonObject data){
    QMutexLocker locker(&d->mutex);
    long long pid = data.find("pid")->toInt(0);
    long long siteid = data.find("siteid")->toInt();
    if(pid!=0 && siteid!=0){
        auto proj = d->root->findByProjectId(pid);
        if(proj!=nullptr){
            auto site = proj->findBySiteId(siteid);

            auto index = createIndex(site->row(),0,site);//site modelindex (job parent modelindex)
            int position = site->childrenCount();
            QString remote = data.find("remote")->toString();//absolute path
            QString local = data.find("local")->toString();//absolute path
            bool is_file = data.find("is_file")->toBool(true);
            long long filesize = data.find("filesize")->toInt();
            bool matched = false;
            if(local.isEmpty()){
                auto req = NetworkManager::getInstance()->request(siteid);
                if(req!=nullptr){
                    const QString projectPath = proj->source() + "/";
                    const QString remotePath = site->destination();//site remote path
                    if(remote.startsWith(remotePath)){
                        QString rRemote = remote.mid(remotePath.length());
                        QString rLocal = req->matchToPath(rRemote,true);
                        if(rLocal.isEmpty()){
                            return ;
                        }
                        local = projectPath + rLocal;
                    }else if(remote==site->destination()){
                        //download to project folder
                        local = projectPath;
                    }else{
                        return ;//not match project
                    }
                }else{
                    return;
                }
                matched = true;
            }
            beginInsertRows(index,position,position);
            auto item = new FileTransferModelItem(FileTransferModelItem::Download,is_file,local,remote,site);
            item->setFilesize(filesize);
            qDebug()<<"filesize:"<<filesize;
            item->setMatchedPath(matched);
            site->appendItem(item);
            endInsertRows();
        }
    }
    d->cond.wakeAll();
}


void FileTransferModel::progress(long long siteid,long long id,float progress){
    emit notifyProgress(siteid,id,progress);
}

void FileTransferModel::onUpdateProgress(long long siteid,long long id,float progress){
    QMutexLocker locker(&d->mutex);
    auto site = d->root->findBySiteId(siteid);
    if(site==nullptr){
        return ;
    }
    int total = site->childrenCount();
    //only search state of failed and doing
    for(int i=0;i<total;i++){
        auto one = site->childAt(i);
        if(one->id()==id){
            one->setProgress(progress);
            QModelIndex index = createIndex(one->row(),FileTransferModel::Status,one);
            emit dataChanged(index,index,QVector<int>(Qt::DisplayRole));
            return ;
        }else if(one->state()==FileTransferModelItem::Wait){
            return ;
        }
    }
}

void FileTransferModel::setItemFailed(long long siteid,long long id,const QString& msg){
    QMutexLocker locker(&d->mutex);
    auto site = d->root->findBySiteId(siteid);
    int jTotal = site->childrenCount();
    for(int k=0;k<jTotal;k++){
        auto one = site->childAt(k);
        if(one->id()==id){
            one->setState(FileTransferModelItem::Failed);
            one->setErrorMsg(msg);
            QModelIndex index = createIndex(one->row(),FileTransferModel::Status,one);
            emit dataChanged(index,index,QVector<int>(Qt::DisplayRole));
            return ;
        }
    }

}

void FileTransferModel::start(int num){
    QMutexLocker locker(&d->mutex);
    d->state = Running;
    QVector<long long> ids;
    int pTotal = d->root->childrenCount();
    for(int i=0;i<pTotal;i++){
        auto proj = d->root->childAt(i);
        int sTotal = proj->childrenCount();
        for(int j=0;j<sTotal;j++){
            ids << proj->childAt(j)->id();
        }
    }
    int count = qMin(num,ids.size());
    int from = d->threads.size();
    for(int i=from;i<count;i++){
        auto thread = new JobThread(ids.at(i),this);
        connect(thread, &JobThread::finished, this, &FileTransferModel::onThreadFinished);
        connect(thread, &JobThread::finishTask, this, &FileTransferModel::onFinishTask);
        connect(thread, &JobThread::errorTask, this, &FileTransferModel::onErrorTask);
        connect(thread, &JobThread::uploadFolder, this, &FileTransferModel::onUploadFolder);
        connect(thread, &JobThread::donwloadFolder, this, &FileTransferModel::onDonwloadFolder);
        thread->start();
        d->threads << thread;
    }
}

void FileTransferModel::stop(){
    QMutexLocker locker(&d->mutex);
    d->state = Stop;
    for(auto one:d->threads){
        one->abort();
    }
}

void FileTransferModel::finish(){
    QMutexLocker locker(&d->mutex);
    d->state = Destory;//set destory state;
    for(auto one:d->threads){
        one->abort();
        one->requestInterruption();
        //one->terminate();
       // one->wait();
    }
    d->cond.wakeAll();
}

void FileTransferModel::abortJob(long long siteid){
    for(auto one:d->threads){
        if(one->id()==siteid){
            one->abort();
        }
    }
}

FileTransferModelItem* FileTransferModel::take(int siteid){
    QMutexLocker locker(&d->mutex);
    if( d->root->childrenCount()==0 || d->state == FileTransferModel::Stop){
        d->cond.wait(&d->mutex,5*1000);
    }
    FileTransferModelItem *item = nullptr;
    while(d->state == FileTransferModel::Stop || (item=this->get(siteid))==nullptr){
        d->cond.wait(&d->mutex,5*1000);

        if(d->state == FileTransferModel::Destory){
            return nullptr;
        }
    }
    //set doing
    item->setState(FileTransferModelItem::Doing);
    QModelIndex index = createIndex(item->row(),FileTransferModel::Status,item);
    emit dataChanged(index,index,QVector<int>(Qt::DisplayRole));
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

FileTransferModelItem* FileTransferModel::rootItem(){
    return d->root;
}

void FileTransferModel::onThreadFinished(){
    JobThread* thread = static_cast<JobThread*>(sender());
    d->threads.removeOne(thread);
    thread->deleteLater();
}

void FileTransferModel::onUploadFolder(long long siteid,long long id,QFileInfoList list,int state){
    this->insertFrontAndRemove(siteid,id,list,static_cast<FileTransferModelItem::State>(state));
}

void FileTransferModel::onDonwloadFolder(long long siteid,long long id,QList<FileItem> list,int state){
    this->insertFrontAndRemove(siteid,id,list,static_cast<FileTransferModelItem::State>(state));
}

void FileTransferModel::onFinishTask(long long siteid,long long id){
    this->removeItem(siteid,id);
}

void FileTransferModel::onErrorTask(long long siteid,long long id,const QString& errorMsg){
    this->setItemFailed(siteid,id,errorMsg);
}

}
