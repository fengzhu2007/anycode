#ifndef RESOURCEMANAGERMODEL_H
#define RESOURCEMANAGERMODEL_H
#include "global.h"
#include <QAbstractItemModel>


namespace ady{
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
    explicit ResourceManagerModel(QObject *parent = nullptr);
    ~ResourceManagerModel();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    ResourceManagerModelPrivate* d;
};

}

#endif // RESOURCEMANAGERMODEL_H