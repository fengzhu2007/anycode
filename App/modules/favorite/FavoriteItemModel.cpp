#include "FavoriteItemModel.h"
namespace ady{

    FavoriteItemModel::FavoriteItemModel(QObject* parent)
        :QAbstractTableModel(parent){

    }

    int FavoriteItemModel::rowCount(const QModelIndex &) const
    {

        return this->m_data.size();
    }

    int FavoriteItemModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return Max;
    }


    QVariant FavoriteItemModel::data(const QModelIndex &index, int role) const
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
        }else if(role==Qt::EditRole){
            const auto &item = m_data.at(index.row());
            if (index.column() == Name){
                return item.name;
            }
        }
        return QVariant();
    }

    bool FavoriteItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if(role==Qt::EditRole){
            if(index.column() ==Name ){
                int row = index.row();
                QString name = value.toString();
                FavoriteRecord item = m_data.at(row);
                if(item.name != name){
                    emit rename(index,name);
                }
                return true;
            }
        }
        return QAbstractTableModel::setData(index,value,role);
    }

    Qt::ItemFlags FavoriteItemModel::flags(const QModelIndex &index) const
    {
        if (!index.isValid())
            return 0;
        Qt::ItemFlags flags = QAbstractItemModel::flags(index);
        if(index.column()==Name){
            flags |= Qt::ItemIsEditable;
        }
        return flags;
    }

    void FavoriteItemModel::setList(QList<FavoriteRecord> data)
    {
        this->m_data = data;
    }

    int FavoriteItemModel::indexOf(long long  id)
    {
        int count = this->rowCount();
        for(int i=0;i<count;i++){
            FavoriteRecord r = this->m_data.at(i);
            if(r.id == id){
                return i;
            }
        }
        return -1;
    }

    FavoriteRecord FavoriteItemModel::getItem(int row)
    {
        return m_data.at(row);
    }

    QVariant FavoriteItemModel::headerData(int section, Qt::Orientation orientation,int role) const
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

    void FavoriteItemModel::updateItem(int row,FavoriteRecord r)
    {
        m_data[row] = r;
        dataChanged(createIndex(row,Name),createIndex(row,Path));
    }

    void FavoriteItemModel::removeItem(int row){
        beginRemoveRows(QModelIndex(),row,row);
        m_data.removeAt(row);
        endRemoveRows();
    }
}
