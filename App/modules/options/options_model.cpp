#include "options_model.h"
#include "option_widget.h"
#include <QIcon>
namespace ady{
OptionsModel::OptionsModel(QObject *parent)
    : QAbstractListModel(parent)
{}

int OptionsModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
    return m_list.size();
}

QVariant OptionsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // FIXME: Implement me!
    if(role==Qt::DisplayRole){
        return at(index.row())->windowTitle();
    }else if(role == Qt::DecorationRole){
        return at(index.row())->windowIcon();
    }
    return QVariant();
}


void OptionsModel::appendItem(OptionWidget* item){
    m_list<<item;
}

void OptionsModel::filter(const QString& text){
    this->beginResetModel();
    auto iter = m_list.begin();
    while(iter!=m_list.end()){
        QString name = (*iter)->windowTitle();
        if(!name.contains(text)){
            m_list.erase(iter);
        }
        iter++;
    }
    this->endResetModel();
}

}
