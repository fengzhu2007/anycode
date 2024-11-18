#ifndef RESOURCEMANAGEICONPROVIDER_H
#define RESOURCEMANAGEICONPROVIDER_H
#include "global.h"
#include <QFileIconProvider>
namespace ady{
class ResourceManagerModelItem;
class ANYENGINE_EXPORT ResourceManageIconProvider
{
public:
    ~ResourceManageIconProvider();
    static ResourceManageIconProvider* getInstance();
    static void destory();
    QIcon icon(ResourceManagerModelItem* item);


private:
    ResourceManageIconProvider();



private:
    static ResourceManageIconProvider* instance;
    QFileIconProvider* provider;
    QIcon m_projectIcon;
    QIcon m_folderIcon;



};
}

#endif // RESOURCEMANAGEICONPROVIDER_H
