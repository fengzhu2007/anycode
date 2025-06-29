#include "frames_model.h"
#include <QDebug>

namespace ady{
class FramesModelPrivate{

public:
    QStringList data;
};

FramesModel::FramesModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    //d = new FramesModelPrivate;
}
FramesModel::~FramesModel(){
    //qDeleteAll(m_data);
    //delete d;
}
QVariant FramesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // FIXME: Implement me!
    return QVariant();
}

int FramesModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return m_data.count();
    // FIXME: Implement me!
}

QVariant FramesModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return QVariant();
    if(role==Qt::DecorationRole){
        auto item = m_data.at(index.row());
        return item.image;
    }
    // FIXME: Implement me!
    return QVariant();
}

void FramesModel::setDatasource(const QStringList& list){
    beginResetModel();
    m_data.clear();
    for(auto one:list){
         FrameItem item ;
         item.image.load(one);
         m_data.append(item);
    }
    endResetModel();
}

const QPixmap& FramesModel::image(int row) const {
    return m_data.at(row).image;
}

}
