#ifndef PROJECTLISTMODEL_H
#define PROJECTLISTMODEL_H
#include <QAbstractListModel>
#include <QList>
#include "storage/ProjectStorage.h"
namespace ady {
    class ProjectListModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
        enum Column {
            Name=0,
            Path,
            Max
        };
        ProjectListModel(QObject* parent);
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        void setData(QList<ProjectRecord> data);
        inline ProjectRecord dataIndex(int index){return this->m_data.at(index);}
        int indexOf(long long  id);
        QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;

    private:
        QList<ProjectRecord>m_data;

    };
}
#endif // PROJECTLISTMODEL_H
