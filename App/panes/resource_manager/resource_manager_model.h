#ifndef RESOURCEMANAGERMODEL_H
#define RESOURCEMANAGERMODEL_H
#include "global.h"
#include <QAbstractItemModel>
#include <QFileInfo>
#include <QJsonArray>
#include <QItemDelegate>



namespace ady{
class ProjectRecord;
class ResourceManagerModelItem;
class ResourceManagerModelPrivate;
class ANYENGINE_EXPORT ResourceManagerModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Column{
        Name=0,
        Max
    };
    enum Action{
        None=0,
        Refresh,
        Insert
    };

    //explicit ResourceManagerModel(QObject *parent = nullptr);
    static ResourceManagerModel* getInstance();
    static void destory();
    ~ResourceManagerModel();

    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
    virtual QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &index) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;

    ResourceManagerModelItem* appendItem(ProjectRecord* project);
    ResourceManagerModelItem* appendItem(const QString& folder);
    QModelIndex insertItem(ResourceManagerModelItem* parent,int type);
    QModelIndex toIndex(ResourceManagerModelItem* item);

    //void removeItem(const QString& path);
    //void updateItem(ResourceManagerModelItem* item);
    //void refreshItem(ResourceManagerModelItem* item);

    void removeItem(ResourceManagerModelItem* item);
    void appendWatchDirectory(const QString& path);
    void removeWatchDirectory(const QString& path);
    QStringList takeWatchDirectory(const QString& path,bool include_children=true);

    ResourceManagerModelItem* find(const QString& path);
    ResourceManagerModelItem* findProject(const QString& path);
    ResourceManagerModelItem* rootItem();

    QModelIndex locate(const QString& path);



    //bool takeWatchDir(const QString& dir);
    QJsonArray toJson();

public slots:
    void onUpdateChildren(QFileInfoList list,const QString& parent,int action);
    void appendItems(QFileInfoList list,ResourceManagerModelItem* parent);
    void refreshItems(QFileInfoList list,ResourceManagerModelItem* parent);
    void onDirectoryChanged(const QString &path);


signals:
    void updateChildren(QFileInfoList list,const QString& parent,int action);
    void insertReady(const QModelIndex& parent,bool isFile);
    void itemsChanged(const QString& path);
    void locateSuccess(const QString& path,bool recursion );
private:
    ResourceManagerModel();
    void findAllExpend(ResourceManagerModelItem* item,QJsonArray& list);

private:
    ResourceManagerModelPrivate* d;
    static ResourceManagerModel* instance;
};





}

#endif // RESOURCEMANAGERMODEL_H
