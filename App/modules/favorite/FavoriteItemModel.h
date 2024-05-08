#ifndef FAVORITEITEMMODEL_H
#define FAVORITEITEMMODEL_H
#include <QAbstractTableModel>
#include "storage/FavoriteStorage.h"
namespace ady {

    class FavoriteItemModel : public QAbstractTableModel
    {
        Q_OBJECT
    public:
        enum Column {
            Name=0,
            Path,
            Max
        };
        FavoriteItemModel(QObject* parent);
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        void setList(QList<FavoriteRecord> data);
        inline FavoriteRecord dataIndex(int index){return this->m_data.at(index);}
        int indexOf(long long  id);
        FavoriteRecord getItem(int row);
        QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;

        void updateItem(int row,FavoriteRecord);
        void removeItem(int row);
    signals:
        void rename(const QModelIndex& index, QString name);
    private:
        QList<FavoriteRecord>m_data;

    };
}
#endif // FAVORITEITEMMODEL_H
