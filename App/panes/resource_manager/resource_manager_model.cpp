#include "resource_manager_model.h"
#include "resource_manager_model_item.h"
#include "resource_manage_icon_provider.h"
#include "storage/project_storage.h"
#include "core/backend_thread.h"
#include "resource_manage_read_folder_task.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/event.h"
#include "core/event_bus/type.h"
#include "common.h"
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QMutex>
#include <QMutexLocker>
#include <QJsonArray>
#include <QJsonObject>
#include <QMimeData>
#include <QDebug>
namespace ady{


ResourceManagerModel* ResourceManagerModel::instance = nullptr;


class ResourceManagerModelPrivate{
public:
    ResourceManagerModelItem* root;
    ResourceManageIconProvider* iconProvider;
    QFileSystemWatcher *watcher;
    QMutex mutex;
};

ResourceManagerModel* ResourceManagerModel::getInstance(){
    if(instance==nullptr){
        instance = new ResourceManagerModel;
    }
    return instance;
}

void ResourceManagerModel::destory(){
    if(instance!=nullptr){
        delete instance;
        instance = nullptr;
    }
}


ResourceManagerModel::ResourceManagerModel()
    : QAbstractItemModel{nullptr}
{
    d = new ResourceManagerModelPrivate();
    d->iconProvider = ResourceManageIconProvider::getInstance();
    d->root = new ResourceManagerModelItem();
    d->watcher = new QFileSystemWatcher(this);
    connect(d->watcher,&QFileSystemWatcher::directoryChanged,this,&ResourceManagerModel::onDirectoryChanged);
    connect(this,&ResourceManagerModel::updateChildren,this,&ResourceManagerModel::onUpdateChildren);
}

ResourceManagerModel::~ResourceManagerModel(){
    ResourceManageIconProvider::destory();
    delete d->root;
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
    }else if(role==Qt::EditRole){
        if (index.column() == Name){
            ResourceManagerModelItem* item = static_cast<ResourceManagerModelItem*>(index.internalPointer());
            return item->data(index.column());
        }
    }else if(role == Qt::ToolTipRole && index.column() == Name){
        ResourceManagerModelItem* item = static_cast<ResourceManagerModelItem*>(index.internalPointer());
        if(item->type() == ResourceManagerModelItem::Project){
            return item->path();
        }
    }

    return QVariant();
}

bool ResourceManagerModel::setData(const QModelIndex &index, const QVariant &value, int role){
    if(role==Qt::EditRole){
        if(index.column() ==Name ){
            QString name = value.toString();
            ResourceManagerModelItem* item = static_cast<ResourceManagerModelItem*>(index.internalPointer());
            const QString path = item->path();
            if(path.isEmpty()){
                //create file or folder
                ResourceManagerModelItem::Type type = item->type();
                const QString parentPath = item->parent()->path();
                QFileInfo fi(parentPath,name);
                if(type==ResourceManagerModelItem::File){
                    QFile file(fi.absoluteFilePath());
                    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {

                        return false;
                    }
                    file.close();
                    //open file to editor
                    auto instance = Publisher::getInstance();
                    QJsonObject data = {
                        {"path",fi.absoluteFilePath()},
                    };
                    instance->post(Type::M_OPEN_EDITOR,data);
                }else if(type==ResourceManagerModelItem::Folder){
                    if(!fi.absoluteDir().mkdir(name)){
                        return false;
                    }

                }else{
                    return false;
                }
                item->setPath(fi.absoluteFilePath());
                item->setTitle(name);
                return true;
            }else if(name!=item->title()){
                //rename
                QString path = item->path();
                QFileInfo fi(path);
                if(fi.exists() && fi.fileName()!=name){
                    //remove path and file
                    const QString dirPath = fi.path();
                    this->removeWatchDirectory(dirPath);
                    auto instance = Publisher::getInstance();
                    QStringList list;
                    if(fi.isFile()){
                        instance->post(new Event(Type::M_WILL_RENAME_FILE,(void*)&path));
                    }else if(fi.isDir()){
                        if(path.endsWith("/")==false){
                            path += "/";
                        }
                        list = this->takeWatchDirectory(path,true);
                        instance->post(new Event(Type::M_WILL_RENAME_FOLDER,(void*)&path));
                    }
                    QString fileName = fi.path() + "/" + name;
                    QDir d(dirPath);
                    bool ret = d.rename(fi.fileName(),name);
                    QPair<QString,QString> pair = {path,fileName};
                    if(ret){
                        item->setTitle(name);
                        item->setPath(fileName);

                        if(fi.isFile()){
                            instance->post(new Event(Type::M_RENAMED_FILE,(void*)&pair));
                        }else if(fi.isDir()){
                            instance->post(new Event(Type::M_RENAMED_FOLDER,(void*)&pair));
                            //refresh item
                            auto task = new ResourceManageReadFolderTask(this,path);
                            task->setType(BackendThreadTask::RefreshFolder);
                            BackendThread::getInstance()->appendTask(task);
                        }
                    }else{
                        pair.second = path;
                        if(fi.isFile()){
                            instance->post(new Event(Type::M_RENAMED_FILE,(void*)&pair));
                        }else if(fi.isDir()){
                            instance->post(new Event(Type::M_RENAMED_FOLDER,(void*)&pair));
                        }
                    }
                    this->appendWatchDirectory(dirPath);
                    return ret;
                }else{
                    return false;
                }
            }

        }
    }
    return QAbstractItemModel::setData(index,value,role);
}

Qt::ItemFlags ResourceManagerModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    flags = flags | Qt::ItemIsDragEnabled ;
    if(index.column()==Name){
        ResourceManagerModelItem* item = static_cast<ResourceManagerModelItem*>(index.internalPointer());
        ResourceManagerModelItem::Type type = item->type();
        if(type==ResourceManagerModelItem::File || type==ResourceManagerModelItem::Folder){
            flags |= Qt::ItemIsEditable;
        }
    }
    return flags;
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
    if(childItem==d->root || childItem==nullptr){
        return QModelIndex();
    }

    ResourceManagerModelItem *parentItem = childItem->parent();
    if (parentItem == d->root || parentItem==nullptr)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

int ResourceManagerModel::rowCount(const QModelIndex &parent) const {
    if (parent.column() > 0){
        return 0;
    }
    ResourceManagerModelItem *parentItem;
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

bool ResourceManagerModel::hasChildren(const QModelIndex &parent)const{
    if(parent.isValid()){
        auto item = static_cast<ResourceManagerModelItem*>(parent.internalPointer());
        if(item!=nullptr){
            switch(item->type()){
            case ResourceManagerModelItem::Solution:
            case ResourceManagerModelItem::Project:
                return true;
            case ResourceManagerModelItem::File:
                return false;
            case ResourceManagerModelItem::Folder:
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

QMimeData* ResourceManagerModel::mimeData(const QModelIndexList &indexes) const
{
    auto data = QAbstractItemModel::mimeData(indexes);
    QStringList* list = new QStringList;
    list->reserve(indexes.size());
    for(auto one:indexes){
        auto item = static_cast<ResourceManagerModelItem*>(one.internalPointer());
        auto type = item->type();
        if(type==ResourceManagerModelItem::File || type==ResourceManagerModelItem::Folder){
            *list<<item->path();
        }
    }
    if(list->size()>0){
        QByteArray b;
        QDataStream in(&b,QIODevice::WriteOnly);
        in<<(qint64)list;
        data->setData(MM_UPLOAD,b);
    }else{
        delete list;
    }
    return data;
}


ResourceManagerModelItem* ResourceManagerModel::appendItem(ProjectRecord* project){
    auto item = new ResourceManagerModelItem(ResourceManagerModelItem::Project,project->name,d->root);
    item->setPath(project->path);
    item->setData(static_cast<void*>(project));
    item->setPid(project->id);
    int position = d->root->childrenCount();
    beginInsertRows(QModelIndex(),position,position);
    d->root->appendItem(item);
    //d->watcher->addPath(project->path);
    this->appendWatchDirectory(project->path);
    endInsertRows();
    //d->root->dump("insert");
    return item;
}

ResourceManagerModelItem* ResourceManagerModel::appendItem(const QString& folder){
    QFileInfo fi(folder);
    auto project = new ProjectRecord();
    project->id = 0l;
    project->name = fi.fileName();
    project->path = folder;
    return this->appendItem(project);
}

QModelIndex ResourceManagerModel::insertItem(ResourceManagerModelItem* parent,int type){
    int row = parent->firstFile();
    auto item = new ResourceManagerModelItem(static_cast<ResourceManagerModelItem::Type>(type),{},parent);
    //qDebug()<<"insertItem:"<<item->type();
    QModelIndex parentIndex = createIndex(parent->row(),0,parent);
    beginInsertRows(parentIndex,row,row);
    parent->insertItem(row,item);
    endInsertRows();
    return createIndex(row,0,item);
}

QModelIndex ResourceManagerModel::toIndex(ResourceManagerModelItem* item){
    return createIndex(item->row(),0,item);
}

void ResourceManagerModel::onUpdateChildren(QFileInfoList list,const QString& parent,int action){
    QMutexLocker locker(&(d->mutex));
    auto item = d->root->findChild(parent);
    if(item!=nullptr){
        if(action==BackendThreadTask::ReadFolder){
            this->appendItems(list,item);

        }else if(action==BackendThreadTask::RefreshFolder){
            this->refreshItems(list,item);
        }else if(action==BackendThreadTask::ReadFolderAndInsertFile){
            this->appendItems(list,item);
            //emit
            QModelIndex parent = createIndex(item->row(),0,item);
            emit insertReady(parent,true);
        }else if(action==BackendThreadTask::ReadFolderAndInsertFolder){
            this->appendItems(list,item);
            //emit
            QModelIndex parent = createIndex(item->row(),0,item);
            emit insertReady(parent,false);
        }
    }
    emit itemsChanged(parent);
}

void ResourceManagerModel::appendItems(QFileInfoList list,ResourceManagerModelItem* parent){
    if(list.size()>0){
        QModelIndex index;
        if(parent!=d->root){
            index = createIndex(parent->row(),0,parent);
        }
        int position = parent->childrenCount();
        beginInsertRows(index,position,position + list.size() - 1);
        parent->appendItems(list);
        endInsertRows();
    }
    parent->setExpanded(true);
}

void ResourceManagerModel::refreshItems(QFileInfoList list,ResourceManagerModelItem* parent){
    QModelIndex index;
    if(parent!=d->root){
        index = createIndex(parent->row(),0,parent);
    }

    int childrenCount = parent->childrenCount();
    //qDebug()<<"path:"<<parent->path()<<";child count:"<<childrenCount;
    if(childrenCount>0){
        beginRemoveRows(index,0,childrenCount - 1);
        auto rows = parent->takeAll();
        endRemoveRows();
        foreach(auto one,rows){
            if(one->type()==ResourceManagerModelItem::Folder){
                this->removeWatchDirectory(one->path());
            }
        }
        qDeleteAll(rows);
    }
    //qDebug()<<"list:"<<list;
    this->appendItems(list,parent);
}



void ResourceManagerModel::removeItem(ResourceManagerModelItem* item){
    QMutexLocker locker(&(d->mutex));
    int row = item->row();
    const QString path = item->path();
    auto parent = item->parent();
    {
        QModelIndex index;
        if(parent!=d->root){
            index = createIndex(parent->row(),0,parent);
        }
        beginRemoveRows(index,row,row);
        parent->takeAt(row);
        endRemoveRows();
        delete item;
    }
    //remove watcher
    if(parent==d->root){
        this->removeWatchDirectory(path);
    }
}

void ResourceManagerModel::appendWatchDirectory(const QString& path){
    //qDebug()<<"addpath"<<path;
    d->watcher->addPath(path);
}

void ResourceManagerModel::removeWatchDirectory(const QString& path){
    //qDebug()<<"removepath"<<path;
    d->watcher->removePath(path);
}

QStringList ResourceManagerModel::takeWatchDirectory(const QString& path,bool include_children){
    auto alllist = d->watcher->directories();
    QStringList list;
    foreach(auto one,alllist){
        if(path==one || (include_children && one.startsWith(path))){
            this->removeWatchDirectory(one);
            list.push_back(one);
        }
    }
    return list;
}

ResourceManagerModelItem* ResourceManagerModel::find(const QString& path){
    QMutexLocker locker(&(d->mutex));
    return d->root->findChild(path);
}

ResourceManagerModelItem* ResourceManagerModel::findProject(const QString& path){
    QMutexLocker locker(&(d->mutex));
    int size = d->root->childrenCount();
    for(int i=0;i<size;i++){
        auto one = d->root->childAt(i);
        if(one->path()==path){
            return one;
        }
    }
    return nullptr;
}

ResourceManagerModelItem* ResourceManagerModel::rootItem(){
    return d->root;
}

QModelIndex ResourceManagerModel::locate(const QString& path){
    auto find = [](ResourceManagerModelItem *parent,const QString& path,bool *ok){
        int total = parent->childrenCount();
        *ok = false;
        if(total>0){
            for(int i=0;i<total;i++){
                auto one = parent->childAt(i);
                auto type = one->type();
                if(type==ResourceManagerModelItem::File && path==one->path()){
                    *ok = true;
                    return one;
                }else if((type==ResourceManagerModelItem::Folder || type==ResourceManagerModelItem::Project ) && path.startsWith(one->path()+"/")){
                    *ok = true;
                    return one;
                }
            }
        }else if(parent->expanded()==false){
            //unkown children
            *ok = true;
        }
        return (ResourceManagerModelItem*)nullptr;
    };

    //auto children = d->root->d->
    auto item = d->root;
    while(true){
        bool ok = false;
        auto next = find(item,path,&ok);
        if(next==nullptr){
            if(ok){
                //read folder
                return createIndex(item->row(),0,item);
            }else{
                break;
            }
        }else if(next->type()==ResourceManagerModelItem::File){
            return createIndex(next->row(),0,next);
        }
        item = next;
    }
    return {};
}

QJsonArray ResourceManagerModel::toJson(){
    QJsonArray projects;
    //find all projects
    int count = d->root->childrenCount();
    for(int i=0;i<count;i++){
        auto child = d->root->childAt(i);
        QJsonArray list;
        //qDebug()<<"path:"<<child->path()<<child->state();
        if(child->state() == ResourceManagerModelItem::Expand){
            list << child->path();
            this->findAllExpend(child,list);
        }
        QJsonObject pro = {
            {"id",child->pid()},
            {"path",child->path()},
            {"opened",list}
        };
        projects<<pro;
    }
    return projects;
}

void ResourceManagerModel::findAllExpend(ResourceManagerModelItem* item,QJsonArray& list){
    int count = item->childrenCount();
    for(int i=0;i<count;i++){
        auto child = item->childAt(i);
        if(child->state() == ResourceManagerModelItem::Expand){
            list << child->path();
            this->findAllExpend(child,list);
        }
    }
}

void ResourceManagerModel::onDirectoryChanged(const QString &path){
    auto task = new ResourceManageReadFolderTask(this,path);
    task->setType(BackendThreadTask::RefreshFolder);
    BackendThread::getInstance()->appendTask(task);
}









}
