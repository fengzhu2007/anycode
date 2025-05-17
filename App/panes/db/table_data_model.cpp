#include "table_data_model.h"
#include <QColor>
#include <QDebug>
namespace ady{

class TableDataModelPrivate{
public:
    QList<QSqlField> fields;
    QList<QList<QVariant>> data;
    QList<int> modifications;

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
    }else if(role==Qt::ForegroundRole){
        const auto &item = d->data.at(index.row());
        auto column = index.column();
        auto val = item.at(column);
        if(val.isNull()){
            return QColor(Qt::lightGray);
        }
    }
    return {};
}

bool TableDataModel::setData(const QModelIndex &index, const QVariant &value, int role){
    if(role==Qt::EditRole){
        auto item = d->data.at(index.row());
        auto col = index.column();
        if(item.at(col)!=value){
            item[index.column()] = value;
            d->data[index.row()] = item;
            if(!d->modifications.contains(index.row())){
                d->modifications.append(index.row());
            }
        }
    }
    return QAbstractItemModel::setData(index,value,role);
}

QVariant TableDataModel::headerData(int section, Qt::Orientation orientation,int role ) const {
    if(orientation == Qt::Horizontal && role==Qt::DisplayRole){
        if(section<d->fields.size()){
            return d->fields.at(section).name();
        }
    }else if(orientation == Qt::Vertical && role==Qt::DisplayRole){
        if(d->modifications.contains(section)){
            return QString::fromUtf8("*");
        }
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

void TableDataModel::appendItem(const QList<QVariant>& item){
    beginInsertRows({},d->data.size(),d->data.size());
    d->data.append(item);
    endInsertRows();

}

void TableDataModel::appendRow(){
    QList<QVariant> item;
    for(int i=0;i<columnCount();i++){
        auto field = d->fields.at(i);
        qDebug()<<"fields"<<field.name()<<field.defaultValue();
        item.append(field.defaultValue());
    }
    this->appendItem(item);
    d->modifications.append(rowCount() - 1);
}

void TableDataModel::updateItem(int row,const QList<QVariant>& item){
    d->data[row] = item;
    auto start = createIndex(row,0);
    auto end = createIndex(row,item.length() - 1);
    dataChanged(start,end,QVector<int>{Qt::DisplayRole,Qt::EditRole});
    this->clearChanged(row);
}

QMap<long long,QList<QVariant>> TableDataModel::changedData() const{
    QMap<long long,QList<QVariant>> data;

    for(auto row:d->modifications){
        data.insert(row,d->data.at(row));
    }
    return data;
}

void TableDataModel::clearChanged(int row){
    if(d->modifications.contains(row)){
        d->modifications.removeAll(row);
        headerDataChanged(Qt::Vertical,row,row);
    }
}

}
