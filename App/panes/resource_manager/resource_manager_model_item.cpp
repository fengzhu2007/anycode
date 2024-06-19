#include "resource_manager_model_item.h"
#include "resource_manager_model.h"
#include <QFileIconProvider>
#include <QList>

static QFileIconProvider* iconProvider = new QFileIconProvider;

namespace ady{

class ResourceManagerModelItemPrivate{
public:
    ResourceManagerModelItem* parent;
    QList<ResourceManagerModelItem*> children;
    ResourceManagerModelItem::Type type;
    QString title;
    void* data;
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

    }else if(d->type==Folder){

    }else if(d->type==File){

    }
    delete d;
}

ResourceManagerModelItem* ResourceManagerModelItem::childAt(int row){
    return d->children.at(row);
}

ResourceManagerModelItem* ResourceManagerModelItem::parent(){
    return d->parent;
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

void ResourceManagerModelItem::setData(void* data){
    d->data = data;
}

QVariant ResourceManagerModelItem::data(int col){
    if(col==ResourceManagerModel::Name){
        return QVariant("");
    }else{
        return QVariant("");
    }
}

}
