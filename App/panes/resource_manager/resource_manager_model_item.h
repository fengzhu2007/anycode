#ifndef RESOURCEMANAGERMODELITEM_H
#define RESOURCEMANAGERMODELITEM_H
#include "global.h"
#include <QVariant>
#include <QIcon>

namespace ady{
class ResourceManagerModelItemPrivate;
class ANYENGINE_EXPORT ResourceManagerModelItem
{
public:
    enum Type {
        Solution=0,//root
        Project,
        Folder,
        File
    };
    explicit ResourceManagerModelItem(Type type=Solution);
    ResourceManagerModelItem(Type type,const QString& title);
    ResourceManagerModelItem(Type type,const QString& title,ResourceManagerModelItem* parent);
    ~ResourceManagerModelItem();
    ResourceManagerModelItem* childAt(int row);
    ResourceManagerModelItem* parent();
    int childrenCount();
    int row();
    void setData(void* data);
    QVariant data(int col);
    QIcon icon(int col);

private:
    ResourceManagerModelItemPrivate* d;
};

}
#endif // RESOURCEMANAGERMODELITEM_H
