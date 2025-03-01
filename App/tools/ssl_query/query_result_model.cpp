#include "query_result_model.h"
#include <QColor>
#include <QDateTime>
#include <QDebug>

namespace ady{
class QueryResultModelPrivate{
public:
    QList<QueryResult> list;
    QDateTime current;
};


class ResultSorting final{
public:
    ResultSorting(QueryResultModel::Column col,bool asc){
        this->col = col;
        this->asc = asc;
    }
    bool operator()(QueryResult& a,QueryResult& b)
    {
        if(this->asc){
            return a.expireDate > b.expireDate;
        }else{
            return a.expireDate < b.expireDate;
        }
        return true;
    }
private:
    bool asc;
    int col;
};

QueryResultModel::QueryResultModel(QObject *parent):QAbstractTableModel(parent) {
    d = new QueryResultModelPrivate();
    d->current = QDateTime::currentDateTime();
}

QueryResultModel::~QueryResultModel(){
    delete d;
    d = nullptr;
}

QVariant QueryResultModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (index.row() >= d->list.size() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto &item = d->list.at(index.row());
        if (index.column() == Domain){
            return item.domain;
        }else if (index.column() == StartDate){
            //return item.size;
            return item.startDate.toString("yyyy-MM-dd hh:mm");
        }else if (index.column() == ExpireDate){
            return item.expireDate.toString("yyyy-MM-dd hh:mm");
        }else if(index.column() == ErrorMsg){
            return item.errorMsg;
        }
    }else if(role==Qt::ForegroundRole){
        if(index.column() == ErrorMsg || index.column() == ExpireDate ){
            const auto &item = d->list.at(index.row());
            if(item.expireDate.isValid()){
                //if(item.expireDate.)
                int ret = d->current.secsTo(item.expireDate);
                if(ret<0){
                    //expired
                    return QColor(Qt::red);
                }else if(ret < 5 * 86400){
                    //will expired
                    return QColor(Qt::darkYellow);
                }
            }else{
                return QColor(Qt::red);
            }
        }
    }
    return QVariant();
}

QVariant QueryResultModel::headerData(int section, Qt::Orientation orientation,int role ) const {
    if (role == Qt::DisplayRole) {
        switch (section) {
        case Domain:
            return tr("Domain");
        case StartDate:
            return tr("Start Date");
        case ExpireDate:
            return tr("Expire Date");
        case ErrorMsg:
            return tr("Status");
        default:
            return QVariant();
        }
    }
    return QVariant();
}

int QueryResultModel::rowCount(const QModelIndex &parent ) const {
    return d->list.size();
}

int QueryResultModel::columnCount(const QModelIndex &parent ) const {
    return Max;
}

void QueryResultModel::setDataSource(const QList<QueryResult>& list){
    beginResetModel();
    d->list = list;
    endResetModel();
}

void QueryResultModel::clear(){
    this->setDataSource({});
}

void QueryResultModel::appendItem(const QueryResult& item){
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    d->list.append(item);
    endInsertRows();
}

void QueryResultModel::removeItem(const QueryResult& item){
    this->removeItem(item.domain);
}

void QueryResultModel::removeItem(const QString& domain){
    for(int i=0;i<d->list.size();i++){
        auto one = d->list.at(i);
        if(one.domain==domain){
            beginRemoveRows(QModelIndex(), i, i);
            d->list.removeAt(i);
            endRemoveRows();
            return ;
        }
    }
}

void QueryResultModel::updateItem(const QueryResult& item){
    for(int i=0;i<d->list.size();i++){
        auto one = d->list.at(i);
        if(one.domain==item.domain){
            d->list.replace(i,item);
            emit dataChanged(createIndex(i,Domain), createIndex(i,ErrorMsg),{Qt::DisplayRole});
            return ;
        }
    }
}

QueryResult QueryResultModel::itemAt(int row){
    return d->list.at(row);
}

QModelIndexList QueryResultModel::search(const QString& text){
    QModelIndexList list;
    for(int i=0;i<d->list.size();i++){
        auto item = d->list.at(i);
        if(item.domain.contains(text)){
            list << createIndex(i,Domain);
        }
    }
    return list;
}

void QueryResultModel::sortAll(){
    if(d->list.size()>0){
        beginResetModel();
        qSort(d->list.begin(),d->list.end(),ResultSorting(ExpireDate,false));
        endResetModel();
    }
}

QStringList QueryResultModel::allDomains(){
    QStringList list;
    for(auto one:d->list){
        list <<one.domain;
    }
    return list;
}


}
