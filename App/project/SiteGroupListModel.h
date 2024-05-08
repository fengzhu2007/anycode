#ifndef SITEGROUPLISTMODEL_H
#define SITEGROUPLISTMODEL_H
#include <QAbstractListModel>
#include <QList>
#include "storage/GroupStorage.h"
namespace ady {
    class SiteGroupListModel : public QAbstractListModel
    {
    public:
        SiteGroupListModel(QObject* parent);
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        void setData(QList<GroupRecord> data);
        inline GroupRecord dataIndex(int index){return this->m_data.at(index);}
        int indexOf(long long  id);

    private:
        QList<GroupRecord>m_data;

    };
}


#endif // SITEGROUPLISTMODEL_H
