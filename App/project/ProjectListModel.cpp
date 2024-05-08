#include "ProjectListModel.h"
namespace ady {
    ProjectListModel::ProjectListModel(QObject* parent)
        :QAbstractListModel(parent)
    {

    }

    int ProjectListModel::rowCount(const QModelIndex &) const
    {

        return this->m_data.size();
    }

    int ProjectListModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return Max;
    }


    QVariant ProjectListModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        if (index.row() >= m_data.size() || index.row() < 0)
            return QVariant();

        if (role == Qt::DisplayRole) {
            const auto &item = m_data.at(index.row());
            if (index.column() == Name){
                return item.name;
            }else if (index.column() == Path){
                return item.path;
            }

        }
        return QVariant();
    }

    void ProjectListModel::setData(QList<ProjectRecord> data)
    {
        this->m_data = data;
    }

    int ProjectListModel::indexOf(long long  id)
    {
        int count = this->rowCount();
        for(int i=0;i<count;i++){
            ProjectRecord r = this->m_data.at(i);
            if(r.id == id){
                return i;
            }
        }
        return -1;
    }

    QVariant ProjectListModel::headerData(int section, Qt::Orientation orientation,int role) const
    {
        if (role == Qt::DisplayRole) {
            switch (section) {
                case Name:
                    return tr("Name");
                case Path:
                    return tr("Path");
                default:
                    return QVariant();
            }
        }
        return QVariant();
    }
}
