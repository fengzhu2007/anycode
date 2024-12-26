#ifndef RESOURCEMANAGEICONPROVIDER_H
#define RESOURCEMANAGEICONPROVIDER_H
#include "global.h"
#include <QFileIconProvider>
#include <QMap>
namespace ady{
class ResourceManagerModelItem;
class ANYENGINE_EXPORT ResourceManageIconProvider
{
public:
    ~ResourceManageIconProvider();
    static ResourceManageIconProvider* getInstance();
    static void destory();
    QIcon icon(ResourceManagerModelItem* item);
    QIcon icon(const QString& suffix);


private:
    ResourceManageIconProvider();



private:
    static ResourceManageIconProvider* instance;
    QFileIconProvider* provider;
    QIcon m_projectIcon;
    QIcon m_folderIcon;
    QMap<QString,QIcon> m_cachelist;



};
}

#endif // RESOURCEMANAGEICONPROVIDER_H
