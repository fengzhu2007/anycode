#include "AddonModel.h"
namespace ady{

    AddonModel::AddonModel(QObject* parent)
        :QAbstractListModel(parent){

    }

    int AddonModel::rowCount(const QModelIndex &) const
    {

        return this->m_data.size();
    }

    int AddonModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return Max;
    }


    QVariant AddonModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        if (index.row() >= m_data.size() || index.row() < 0)
            return QVariant();

        if (role == Qt::DisplayRole) {
            const auto &item = m_data.at(index.row());
            if (index.column() == Title){
                return item.title;
            }else if (index.column() == File){
                QString path = QString("addon/%1.").arg(item.file);



#ifdef Q_OS_WIN
                path.append("dll");
#elif defined Q_OS_MAC
                path.append("dylib");
#else
                path.append("so");
#endif

                return path;
            }else if(index.column()==Status){
                return item.status==0?tr("Disabled"):tr("Available");
            }
        }
        return QVariant();
    }

    void AddonModel::setList(QList<AddonRecord> data)
    {
        this->m_data = data;
    }

    int AddonModel::indexOf(long long  id)
    {
        int count = this->rowCount();
        for(int i=0;i<count;i++){
            AddonRecord r = this->m_data.at(i);
            if(r.id == id){
                return i;
            }
        }
        return -1;
    }

    QVariant AddonModel::headerData(int section, Qt::Orientation orientation,int role) const
    {
        if (role == Qt::DisplayRole) {
            switch (section) {
                case Title:
                    return tr("Title");
                case File:
                    return tr("Path");
                case Status:
                    return tr("Status");
                default:
                    return QVariant();
            }
        }
        return QVariant();
    }
}
