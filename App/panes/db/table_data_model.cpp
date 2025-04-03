#include "table_data_model.h"
#include <QDebug>
namespace ady{

class TableDataModelPrivate{
public:
    QList<QSqlField> fields;
    QList<QList<QVariant>> data;

};

TableDataModel::TableDataModel(QObject* parent):QAbstractListModel(parent) {
    d = new TableDataModelPrivate;
}

TableDataModel::~TableDataModel(){
    delete d;
}

int TableDataModel::rowCount(const QModelIndex &parent ) const {
    return d->data.size();
}

int TableDataModel::columnCount(const QModelIndex &parent ) const {
    return d->fields.size();
}

QVariant TableDataModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();
    if(role==Qt::DisplayRole || role==Qt::EditRole){
        const auto &item = d->data.at(index.row());
        auto column = index.column();
        if(column<item.size()){
            return item.at(column);
        }
    }
    return {};
}

QVariant TableDataModel::headerData(int section, Qt::Orientation orientation,int role ) const {
    if(orientation == Qt::Horizontal && role==Qt::DisplayRole){
        if(section<d->fields.size()){
            return d->fields.at(section).name();
        }
    }else if(orientation == Qt::Vertical && role==Qt::DisplayRole){

    }
    return {};
}

Qt::ItemFlags TableDataModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    flags = flags | Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
    return flags;
}

void TableDataModel::setDatasource(const QList<QSqlField>& fields,const QList<QList<QVariant>>& data){
    beginResetModel();
    d->fields = fields;
    d->data = data;
    endResetModel();
}

}
