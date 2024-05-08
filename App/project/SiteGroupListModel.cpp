#include "SiteGroupListModel.h"

#include <QDebug>
namespace ady {
    SiteGroupListModel::SiteGroupListModel(QObject* parent)
        :QAbstractListModel(parent)
    {

    }

    int SiteGroupListModel::rowCount(const QModelIndex &) const
    {

        return this->m_data.size();
    }

    QVariant SiteGroupListModel::data(const QModelIndex &index, int role) const
    {
        if (role != Qt::DisplayRole) {
            return QVariant();
        }
        int size = this->m_data.size();
        //qDebug()<<"row:"<<index.row()<<";col:"<<index.column()<<";size:"<<size;
        int row = index.row();
        if(row >=0 && row < size){
            return this->m_data.at(row).name;
        }else{
            return QVariant();
        }
    }

    void SiteGroupListModel::setData(QList<GroupRecord> data)
    {
        this->m_data = data;
    }

    int SiteGroupListModel::indexOf(long long  id)
    {
        int count = this->rowCount();
        for(int i=0;i<count;i++){
            GroupRecord r = this->m_data.at(i);
            if(r.id == id){
                return i;
            }
        }
        return -1;
    }
}
