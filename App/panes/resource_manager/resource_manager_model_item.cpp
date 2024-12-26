#include "resource_manager_model_item.h"
#include "resource_manager_model.h"
#include "storage/project_storage.h"
#include <QFileIconProvider>
#include <QList>
#include <QDebug>

static QFileIconProvider* iconProvider = new QFileIconProvider;

namespace ady{

class ResourceManagerModelItemPrivate{
public:
    ResourceManagerModelItem* parent;
    QList<ResourceManagerModelItem*> children;
    bool expanded=false;
    ResourceManagerModelItem::State state=ResourceManagerModelItem::Collapse;
    ResourceManagerModelItem::Type type;
    long long pid=0;
    QString title;
    QString path;
    void* data;
    QStringList opendlist;
};


ResourceManagerModelItem::ResourceManagerModelItem(Type type)
{
    d = new ResourceManagerModelItemPrivate();
    d->type = type;
    d->data = nullptr;
    d->parent = nullptr;
}

ResourceManagerModelItem::ResourceManagerModelItem(Type type,const QString& title){
    d = new ResourceManagerModelItemPrivate();
    d->type = type;
    d->title = title;
    d->data = nullptr;
    d->parent = nullptr;
}

ResourceManagerModelItem::ResourceManagerModelItem(Type type,const QString& title,ResourceManagerModelItem* parent){
    d = new ResourceManagerModelItemPrivate();
    d->type = type;
    d->title = title;
    d->data = nullptr;
    d->parent = parent;
}

ResourceManagerModelItem::~ResourceManagerModelItem(){
    if(d->type==Project){
        delete (static_cast<ProjectRecord*>(d->data));
    }
    qDeleteAll(d->children);
    delete d;
}

ResourceManagerModelItem* ResourceManagerModelItem::childAt(int row){
    return d->children.at(row);
}

ResourceManagerModelItem* ResourceManagerModelItem::parent(){
    return d->parent;
}

ResourceManagerModelItem* ResourceManagerModelItem::takeAt(int row){
    if(row>=0 && row<this->childrenCount()){
        return d->children.takeAt(row);
    }
    return nullptr;
}

ResourceManagerModelItem* ResourceManagerModelItem::findChild(const QString& path){
    foreach(auto one,d->children){
        const QString oPath = one->path();
        if(oPath==path){
            return one;
        }else if(path.startsWith(oPath + "/")){
            Type type = one->type();
            if(type==ResourceManagerModelItem::Folder || type==ResourceManagerModelItem::Project){
                auto ret = one->findChild(path);
                if(ret!=nullptr){
                    return ret;
                }
            }
        }
    }
    return nullptr;
}

QList<ResourceManagerModelItem*> ResourceManagerModelItem::takeAll(){
    auto list = d->children;
    d->children.clear();
    return list;
}

int ResourceManagerModelItem::childrenCount(){
    return d->children.size();
}

int ResourceManagerModelItem::row(){
    int i = -1;
    if(d->parent!=nullptr){
        foreach(auto one,d->parent->d->children){
            i+=1;
            if(one==this){
                break;
            }
        }
    }
    return i;
}

int ResourceManagerModelItem::firstFile(){
    int i=0;
    for(auto one:d->children){
        if(one->type()==File){
            break;
        }else{
            i += 1;
        }
    }
    return i;
}

void ResourceManagerModelItem::setTitle(const QString& title){
    d->title = title;
}

void ResourceManagerModelItem::setPath(const QString& path,bool recursive){
    d->path = path;
    if(d->type==Folder){
        foreach(auto one,d->children){
            if(path.endsWith("/")==false){
                one->setPath(path+"/"+one->title(),recursive);
            }else{
                one->setPath(path + one->title(),recursive);
            }
        }
    }
}

void ResourceManagerModelItem::setPid(long long pid){
    d->pid = pid;
}

const QString ResourceManagerModelItem::path(){
    return d->path;
}

const QString ResourceManagerModelItem::title(){
    return d->title;
}

long long ResourceManagerModelItem::pid(){
    if(d->type==Folder || d->type==File){
        return d->parent->pid();
    }else{
        return d->pid;
    }
}

void ResourceManagerModelItem::setData(void* data){
    d->data = data;
}

QVariant ResourceManagerModelItem::data(int col){
    if(col==ResourceManagerModel::Name){
        return QVariant(d->title);
    }else{
        return QVariant();
    }
}


QIcon ResourceManagerModelItem::icon(int col){

    return QIcon();
}

void ResourceManagerModelItem::setDataSource(QFileInfoList list){
    qDeleteAll(d->children);
    this->appendItems(list);
}

void ResourceManagerModelItem::appendItems(QFileInfoList list){
    foreach(auto one , list){
        Type type = one.isFile()?File:Folder;
        auto item = new ResourceManagerModelItem(type,one.fileName(),this);
        item->setPath(one.absoluteFilePath());
        d->children.push_back(item);

    }
}

void ResourceManagerModelItem::appendItem(QFileInfo one){
    this->appendItems(QFileInfoList{one});
}

void ResourceManagerModelItem::appendItem(ResourceManagerModelItem* item){
    d->children.push_back(item);
}

void ResourceManagerModelItem::insertItem(int row,ResourceManagerModelItem* item){
    d->children.insert(row,item);
}


bool ResourceManagerModelItem::expanded(){
    return d->expanded;
}

void ResourceManagerModelItem::setExpanded(bool expanded){
    d->expanded = expanded;
}

ResourceManagerModelItem::State ResourceManagerModelItem::state(){
    return d->state;
}

void ResourceManagerModelItem::setState(State state){
    d->state = state;
}
ResourceManagerModelItem::Type ResourceManagerModelItem::type(){
    return d->type;
}

void ResourceManagerModelItem::setOpenList(const QStringList& list){
    d->opendlist = list;
}

QStringList& ResourceManagerModelItem::openList(){
    return d->opendlist;
}

void ResourceManagerModelItem::removeOpenList(const QString& path){
    d->opendlist.removeOne(path);
}

void ResourceManagerModelItem::dump(const QString& prefix){
    qDebug()<<prefix<<this<<d->path;
    foreach(auto one,d->children){
        one->dump(prefix+"--");
    }
}
}
