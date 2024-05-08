#ifndef MANAGERMODEL_H
#define MANAGERMODEL_H


#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>


namespace ady {


    class ManagerTreeItem;
    class SiteRecord;
    class GroupRecord;
    class ProjectRecord;

    //! [0]
    class ManagerTreeModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        explicit ManagerTreeModel(QList<ManagerTreeItem*> lists , QObject *parent = 0);
        ~ManagerTreeModel();

        QVariant data(const QModelIndex &index, int role) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
        QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex &index) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        void setData(QList<ManagerTreeItem*> lists);

        inline ManagerTreeItem* getRootItem(){return this->rootItem;}

        void notifyDataChanged(ManagerTreeItem* item);

        void appendRow(SiteRecord* record);
        void appendRow(int pid,GroupRecord* record);
        void appendRow(ProjectRecord* record);

        void removeRow(SiteRecord* record);

        //void appendItem(ManagerTreeItem* item);
        //void updateItem(ManagerTreeItem* item);

    private:
        //void setupModelData(const QStringList &lines, ManagerTreeItem *parent);

        ManagerTreeItem *rootItem;
    };
}

#endif // MANAGERMODEL_H
