#ifndef SELECT_MODEL_H
#define SELECT_MODEL_H

#include "global.h"
#include <QAbstractListModel>

namespace ady{

template<typename T>
class ANYENGINE_EXPORT SelectModel : public QAbstractListModel
{
public:
    enum Column{
        Name=0,
        Max
    };
    explicit SelectModel(QObject* parent=nullptr):QAbstractListModel(parent){

    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override{
        return m_data.size();
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override{
        return Max;
    }
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override{
        if (!index.isValid())
            return QVariant();

        if (index.row() >= m_data.size() || index.row() < 0)
            return QVariant();

        if (role == Qt::DisplayRole) {
            const auto &item = m_data.at(index.row());
            if (index.column() == Name){
                return item.second;
            }
        }
        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override{
        if (role == Qt::DisplayRole) {
            switch (section) {
            case Name:
                return tr("Name");
            }
        }
        return QVariant();
    }

    void setDataSource(const QList<QPair<T,QString>>& list){
        beginResetModel();
        m_data = list;
        endResetModel();
    }

    QList<QPair<T,QString>>& toList() const{
        return m_data;
    }

    T value(int i){
        return m_data.at(i).first;
    }

private:
    QList<QPair<T,QString>> m_data;
};

}

#endif // SELECT_MODEL_H
