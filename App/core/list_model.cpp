#include "list_model.h"


namespace ady{

template <typename T>
ListModel<T>::ListModel(QObject *parent)
    : QAbstractListModel(parent)
{}


template <typename T>
int ListModel<T>::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_list.size();
}

template <typename T>
void ListModel<T>::setDataSource(QList<T> list){
    beginResetModel();
    m_list = list;
    endResetModel();
}

template <typename T>
void ListModel<T>::removeItem(int row){
    return m_list.removeAt(row);
}

template <typename T>
T ListModel<T>::itemAt(int row){
    return m_list.at(row);
}


}
