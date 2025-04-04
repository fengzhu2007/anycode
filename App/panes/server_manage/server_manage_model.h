#ifndef SERVERMANAGEMODEL_H
#define SERVERMANAGEMODEL_H
#include "global.h"
#include "interface/file_item.h"
#include <QAbstractItemModel>

namespace ady{
class ServerManageModelItemPrivate;
class ServerManageModelItem{
public:
    enum Type{
        Solution=0,
        Project,
        Server,
        Folder,
        File
    };
    ServerManageModelItem();
    ServerManageModelItem(Type type,long long id,const QString& name,ServerManageModelItem* parent);//project
    ServerManageModelItem(Type type,long long id,const QString& name,const QString& path,ServerManageModelItem* parent);// site
    ServerManageModelItem(const FileItem& item,ServerManageModelItem* parent);

    ~ServerManageModelItem();

    int childrenCount();
    int row();
    void appendItems(QList<ServerManageModelItem*> items);
    void appendItem(ServerManageModelItem* item);
    void insertItem(int row,ServerManageModelItem* item);
    ServerManageModelItem* parent();
    ServerManageModelItem* childAt(int i);
    ServerManageModelItem* take(int i);
    QList<ServerManageModelItem*> takeAll();
    Type type();
    QString name() const;
    QString path() const;
    void setName(const QString& name);
    void setPath(const QString& path);
    bool expanded();
    void setExpanded(bool expanded);
    long long sid();
    long long pid();
    bool isLoading();
    void setLoading(bool state);
    FileItem* data();
    ServerManageModelItem* findChild(long long id,bool project=false);
    ServerManageModelItem* findChild(const QString& path);

    void setAddonType(const QString& type);
    QString addonType();


private:
    ServerManageModelItemPrivate* d;
};

class SiteRecord;
class ServerManageModelPrivate;
class ANYENGINE_EXPORT ServerManageModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Column {
        Name=0,
        Max
    };
    explicit ServerManageModel(QObject *parent = nullptr);
    ~ServerManageModel();
    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    // Basic functionality:
    QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    void appendItem(ServerManageModelItem* parent,ServerManageModelItem* item);
    void appendItems(QList<FileItem> list,ServerManageModelItem* parent);
    void refreshItems(QList<FileItem> list,ServerManageModelItem* parent);
    void openProject(long long id,const QString& name);

    void addSite(const SiteRecord& site);
    void updateSite(const SiteRecord& site);

    void removeItem(ServerManageModelItem* item);
    void removeProject(long long id);
    void removeSite(long long id);

    ServerManageModelItem* find(const QString& path);
    ServerManageModelItem* find(long long id,bool project=false);//find server
    ServerManageModelItem* find(long long id,const QString& path);
    ServerManageModelItem* rootItem();

    //void dataChanged();
    void changeItem(ServerManageModelItem* item);

signals:
    void rename(ServerManageModelItem* item,const QString& newName);

private:
    ServerManageModelPrivate * d;
};
}
#endif // SERVERMANAGEMODEL_H
