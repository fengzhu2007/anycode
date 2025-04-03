#include "table_field_model.h"

namespace ady{
class TableFieldModelPrivate{
public:
    QList<QSqlField>data;
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
            return field.name();
        }else if(column==Type){
            return field.type();
        }else if(column==Length){
            return field.length();
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

void TableFieldModel::setDatasource(const QList<QSqlField>& data){
    beginResetModel();
    d->data = data;
    endResetModel();
}

}
