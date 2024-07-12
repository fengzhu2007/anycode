#ifndef RESOURCEMANAGERMODELITEM_H
#define RESOURCEMANAGERMODELITEM_H
#include "global.h"
#include <QVariant>
#include <QIcon>
#include <QFileInfo>

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
    ResourceManagerModelItem* takeAt(int row);
    ResourceManagerModelItem* findChild(const QString& path);
    QList<ResourceManagerModelItem*> takeAll();
    int childrenCount();
    int row();
    void setTitle(const QString& title);
    void setPath(const QString& path,bool recursive=false);
    const QString path();
    const QString title();
    void setData(void* data);
    QVariant data(int col);
    QIcon icon(int col);
    void setDataSource(QFileInfoList list);
    void appendItems(QFileInfoList list);
    void appendItem(QFileInfo one);
    void appendItem(ResourceManagerModelItem* item);
    bool expanded();
    void setExpanded(bool expanded);
    Type type();
    void dump(const QString& prefix={});


private:
    ResourceManagerModelItemPrivate* d;
};

}
#endif // RESOURCEMANAGERMODELITEM_H
