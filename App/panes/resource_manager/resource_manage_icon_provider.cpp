#include "resource_manage_icon_provider.h"
#include "resource_manager_model_item.h"


namespace ady{

ResourceManageIconProvider* ResourceManageIconProvider::instance = nullptr;


ResourceManageIconProvider::~ResourceManageIconProvider(){
    delete provider;
}

ResourceManageIconProvider* ResourceManageIconProvider::getInstance(){
    if(ResourceManageIconProvider::instance==nullptr){
       ResourceManageIconProvider::instance = new ResourceManageIconProvider();
    }
    return ResourceManageIconProvider::instance;
}

void ResourceManageIconProvider::destory(){
    if(ResourceManageIconProvider::instance!=nullptr){
       delete ResourceManageIconProvider::instance;
    }
}

QIcon ResourceManageIconProvider::icon(ResourceManagerModelItem* item){
    switch(item->type()){
    case ResourceManagerModelItem::Project:
        //return provider->icon(QFileIconProvider::Network);
       return QIcon(":/Resource/icons/DocumentsFolder_16x.svg");
    case ResourceManagerModelItem::Folder:
        return provider->icon(QFileIconProvider::Folder);
    default:
    {
        const QString path = item->path();
        if(path.isEmpty()){
            return provider->icon(QFileIconProvider::File);
        }else{
            return provider->icon(QFileInfo(item->path()));
        }
    }

    }
}

ResourceManageIconProvider::ResourceManageIconProvider()
{
    provider = new QFileIconProvider();

}


}
