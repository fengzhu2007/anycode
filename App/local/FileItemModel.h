#ifndef FILEITEMMODEL_H
#define FILEITEMMODEL_H
#include "../global.h"
#include "file_item.h"
#include <QAbstractTableModel>
#include <QList>
#include <QFileIconProvider>
namespace ady {
    class LocalFileItemSorting;
    class ANYENGINE_EXPORT FileItemModel : public QAbstractTableModel
    {
        Q_OBJECT
    public:
        enum Column {
            Name=0,
            Size,
            ModifyTime,
            Max
        };

        FileItemModel(QObject *parent = nullptr);
        ~FileItemModel();

        virtual QVariant data(const QModelIndex &index, int role) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;



        void updateAll(QList<FileItem> data);
        void removeItem(FileItem item);
        void insertItem(int index,FileItem);
        void updateItem(int index,FileItem);
        FileItem getItem(int row);

        void readDir(const QString& dir);

        void setList(QList<FileItem> data);

    public slots:
        void sortList(int col, Qt::SortOrder order);


    private:
        QList<FileItem> m_data;
        QFileIconProvider* m_iconProvider;
        LocalFileItemSorting* m_sorting;

    };

}
#endif // FILEITEMMODEL_H
