#ifndef LIST_MODEL_H
#define LIST_MODEL_H

#include <QAbstractListModel>

namespace ady{
template<class T>
class ListModel : public QAbstractListModel
{

public:
    explicit ListModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override{
        return T::header(section,role);
    }

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override{
        T one = m_list.at(index.row());
        return one.data(index.column(),role);
    }

    void setDataSource(QList<T> list);

    void appendItem(T one){
        m_list << one;
    }

    void appendItems(QList<T> items){
        for(auto one:items){
            m_list<<one;
        }
    }

    void insertItem(int row,T item){
        m_list.insert(row,item);
    }

    void removeItem(int row);

    T itemAt(int row);

    inline QList<T> toList(){
        return m_list;
    }

private:
    QList<T> m_list;
};
}

#endif // LIST_MODEL_H
