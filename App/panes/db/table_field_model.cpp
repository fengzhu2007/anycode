#include "table_field_model.h"

namespace ady{
class TableFieldModelPrivate{
public:
    QList<TableField>data;
};

TableFieldModel::TableFieldModel(QObject* parent):QAbstractListModel(parent){
    d = new TableFieldModelPrivate;

}

TableFieldModel::~TableFieldModel(){
    delete d;
}

int TableFieldModel::rowCount(const QModelIndex &parent ) const {
    return d->data.size();
}

int TableFieldModel::columnCount(const QModelIndex &parent) const {
    return Max;
}

QVariant TableFieldModel::data(const QModelIndex &index, int role ) const {
    if(role==Qt::DisplayRole){
        int column = index.column();
        auto field = d->data.at(index.row());
        if(column==Name){
            return field.name;
        }else if(column==Type){
            return field.type;
        }else if(column==Length){
            return field.length;
        }else if(column==PrimaryKey){
            return field.primaryKey?1:0;
        }
    }
    return {};
}

QVariant TableFieldModel::headerData(int section, Qt::Orientation orientation,int role) const {
    if(role==Qt::DisplayRole){
        if(section==Name){
            return tr("Name");
        }else if(section==Type){
            return tr("Type");
        }else if(section==Length){
            return tr("Length");
        } if(section==PrimaryKey){
            return tr("Primary Key");
        }
    }
    return {};
}

void TableFieldModel::setDatasource(const QList<TableField>& data){
    beginResetModel();
    d->data = data;
    endResetModel();
}

void TableFieldModel::appendItem(const TableField& field){
    beginResetModel();
    d->data.append(field);
    endResetModel();
}

void TableFieldModel::updateItem(const TableField& field){
    for(int i=0;i<d->data.length();i++){
        auto one = d->data.at(i);
        if(one.id==field.id){
            auto start = this->index(i,Name);
            auto end = this->index(i,PrimaryKey);
            d->data[i] = field;
            emit dataChanged(start,end,QVector<int>{Qt::DisplayRole});
            return ;
        }
    }
}

void TableFieldModel::removeItem(int row){
    beginResetModel();
    d->data.removeAt(row);
    endResetModel();
}

QList<TableField>& TableFieldModel::fields() const{
    return d->data;
}

TableField TableFieldModel::at(int row) const {
    return d->data.at(row);
}


}
