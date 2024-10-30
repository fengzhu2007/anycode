#include "site_type_list_model.h"

namespace ady {
    SiteTypeListModel::SiteTypeListModel(QObject* parent)
        :QAbstractListModel(parent)
    {

    }

    int SiteTypeListModel::rowCount(const QModelIndex &) const
    {
        return this->m_data.size();
    }

    QVariant SiteTypeListModel::data(const QModelIndex &index, int role) const
    {
        if (role != Qt::DisplayRole) {
            return QVariant();
        }
        int size = this->m_data.size();
        int row = index.row();
        if(row >=0 && row < size){
            return this->m_data.at(row).label;
        }else{
            return QVariant();
        }
    }

    void SiteTypeListModel::setData(QList<AddonRecord> data)
    {
        this->m_data = data;
    }

    int SiteTypeListModel::indexOf(QString name)
    {
        int count = this->rowCount();
        for(int i=0;i<count;i++){
            AddonRecord r = this->m_data.at(i);
            if(r.name==name){
                return i;
            }
        }
        return -1;
    }
}
