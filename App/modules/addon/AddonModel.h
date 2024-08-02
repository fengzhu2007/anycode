#ifndef ADDONMODEL_H
#define ADDONMODEL_H
#include <QAbstractTableModel>
#include "storage/addon_storage.h"
namespace ady {

    class AddonModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
        enum Column {
            Title=0,
            File,
            Status,
            Max
        };
        AddonModel(QObject* parent);
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        void setList(QList<AddonRecord> data);
        inline AddonRecord dataIndex(int index){return this->m_data.at(index);}
        int indexOf(long long  id);
        QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;

    private:
        QList<AddonRecord>m_data;

    };
}
#endif // ADDONMODEL_H
