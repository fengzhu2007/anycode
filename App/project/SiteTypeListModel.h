#ifndef SITETYPELISTMODEL_H
#define SITETYPELISTMODEL_H
#include <QAbstractListModel>
#include <QList>
#include "storage/addon_storage.h"
namespace ady {
    class SiteTypeListModel : public QAbstractListModel
    {
    public:
        SiteTypeListModel(QObject* parent);
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        void setData(QList<AddonRecord> data);
        inline AddonRecord dataIndex(int index){return this->m_data.at(index);}
        int indexOf(QString name);

    private:
        QList<AddonRecord>m_data;
    };
}
#endif // SITETYPELISTMODEL_H
