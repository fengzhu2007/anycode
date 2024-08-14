#include "search_scope_model.h"


namespace ady{


class SearchScopeModelPrivate{
public:
    QList<QPair<QString,QString>> list;
};

SearchScopeModel::SearchScopeModel(QObject* parent)
    :QAbstractListModel(parent)
{
    d = new SearchScopeModelPrivate;
}

SearchScopeModel::~SearchScopeModel(){
    delete d;
}


int SearchScopeModel::rowCount(const QModelIndex &) const
{

    return d->list.size();
}

int SearchScopeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return Max;
}


QVariant SearchScopeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= d->list.size() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto &item = d->list.at(index.row());
        if (index.column() == Name){
            return item.second;
        }
    }
    return QVariant();
}



QVariant SearchScopeModel::headerData(int section, Qt::Orientation orientation,int role) const
{
    if (role == Qt::DisplayRole) {
        switch (section) {
        case Name:
            return tr("Name");
        }
    }
    return QVariant();
}

void SearchScopeModel::setDataSource(QList<QPair<QString,QString>> list){
    beginResetModel();
    d->list = list;
    endResetModel();
}

QList<QPair<QString,QString>> SearchScopeModel::toList(){
    return d->list;
}

QString SearchScopeModel::value(int i){
    return d->list.at(i).first;
}

}
