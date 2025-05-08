#include "table_index_model.h"
#include <QDebug>

namespace ady{
class TableIndexModelPrivate{
public:
    QList<TableIndex>data;
};

TableIndexModel::TableIndexModel(QObject* parent):QAbstractListModel(parent){
    d = new TableIndexModelPrivate;

}

TableIndexModel::~TableIndexModel(){
    delete d;
}

int TableIndexModel::rowCount(const QModelIndex &parent ) const {
    return d->data.size();
}

int TableIndexModel::columnCount(const QModelIndex &parent) const {
    return Max;
}

QVariant TableIndexModel::data(const QModelIndex &index, int role ) const {
    if(role==Qt::DisplayRole){
        int column = index.column();
        auto one = d->data.at(index.row());
        if(column==Name){
            return one.name;
        }else if(column==FeildList){
            auto list = one.fields;
            QStringList arr;
            for(auto one:list){
                arr.append(one.name);
            }
            return arr.join(",");
        }
    }
    return {};
}

QVariant TableIndexModel::headerData(int section, Qt::Orientation orientation,int role) const {
    if(role==Qt::DisplayRole){
        if(section==Name){
            return tr("Name");
        }else if(section==FeildList){
            return tr("Fields");
        }
    }
    return {};
}

void TableIndexModel::setDatasource(const QList<TableIndex>& data){
    beginResetModel();
    d->data = data;
    endResetModel();
}

void TableIndexModel::appendItem(const TableIndex& field){
    beginResetModel();
    d->data.append(field);
    endResetModel();
}

void TableIndexModel::updateItem(const TableIndex& index){
    for(int i=0;i<d->data.length();i++){
        auto one = d->data.at(i);
        if(one.id==index.id){
            auto start = this->index(i,Name);
            auto end = this->index(i,FeildList);
            d->data[i] = index;
            qDebug()<<"index"<<index.id<<index.name<<d->data[i].fields.length();
            emit dataChanged(start,end,QVector<int>{Qt::DisplayRole});
            return ;
        }
    }
}

void TableIndexModel::removeItem(int row){
    beginResetModel();
    d->data.removeAt(row);
    endResetModel();
}

QList<TableIndex>& TableIndexModel::indexes() const{
    return d->data;
}

TableIndex TableIndexModel::at(int row) const {
    return d->data.at(row);
}


}
