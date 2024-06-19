#include "resource_manage_icon_provider.h"


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

    return provider->icon(QFileIconProvider::File);

    /*return m_iconProvider->icon(QFileIconProvider::Network);
}else{
    if(task->type==0){
       //file
       return m_iconProvider->icon(QFileInfo(item->dataName()));
    }else if(task->type==1){
       //dir
       return m_iconProvider->icon(QFileIconProvider::Folder);
    }else{
       return m_iconProvider->icon(QFileIconProvider::File);
    }*/
}

ResourceManageIconProvider::ResourceManageIconProvider()
{
    provider = new QFileIconProvider();

}


}
