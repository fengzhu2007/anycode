#ifndef REMOTEFILEITEMMODEL_H
#define REMOTEFILEITEMMODEL_H
#include "../global.h"
#include "local/file_item.h"
#include <QAbstractTableModel>
#include <QList>
#include <QFileIconProvider>
#include <QMutex>
namespace ady {

    class FileItemSorting;
    class ANYENGINE_EXPORT RemoteFileItemModel : public QAbstractTableModel
    {
        Q_OBJECT
    public:
        enum Column {
            Name=0,
            Size,
            ModifyTime,
            Permission,
            Max
        };

        RemoteFileItemModel(long long id,QObject *parent = nullptr);
        ~RemoteFileItemModel();

        QVariant data(const QModelIndex &index, int role) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;



        void updateAll(QList<FileItem> data);
        void removeItem(FileItem item);
        void insertItem(int index,FileItem);
        void updateItem(int index,FileItem);
        FileItem getItem(int row);

        void setList(QList<FileItem> data);
        //void sort(Column col,bool asc);


    public slots:
        void sortList(int col, Qt::SortOrder order);

    signals:
        void cellEditing(const QModelIndex &index,QString newString);

    private:
        QList<FileItem> m_data;
        QFileIconProvider* m_iconProvider;
        FileItemSorting* m_sorting;
        QMutex mutex;
        long long id;
    };

}
#endif // REMOTEFILEITEMMODEL_H
