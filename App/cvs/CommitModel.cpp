#include "CommitModel.h"
#include <QColor>
#include <QApplication>
#include <QDebug>
namespace ady {

CommitItemDelegate::CommitItemDelegate(QObject* parent)
    :QStyledItemDelegate(parent)
{
    m_devIcon.load(QString::fromUtf8(":/img/Resource/flag_dev.svg"));
    m_prodIcon.load(QString::fromUtf8(":/img/Resource/flag_prod.svg"));
    m_otherIcon.load(QString::fromUtf8(":/img/Resource/flag_other.svg"));
}

void CommitItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int col = index.column();
    if(col == CommitModel::Flags){
        QStyle* style =  QApplication::style();
        int value = index.data().toInt();
        if((value & cvs::Commit::Dev)==cvs::Commit::Dev){
            QRect rc(option.rect.x(),option.rect.y(),m_boxWidth,option.rect.height());
            style->drawItemPixmap(painter,rc,Qt::AlignCenter,m_devIcon);
        }
        if((value & cvs::Commit::Prod)==cvs::Commit::Prod){
            QRect rc(option.rect.x() + m_boxWidth,option.rect.y(),m_boxWidth,option.rect.height());
            style->drawItemPixmap(painter,rc,Qt::AlignCenter,m_prodIcon);
        }
        if((value & cvs::Commit::Other)==cvs::Commit::Other){
            QRect rc(option.rect.x() + m_boxWidth * 2,option.rect.y(),m_boxWidth,option.rect.height());
            style->drawItemPixmap(painter,rc,Qt::AlignCenter,m_otherIcon);
        }
    }
    QStyledItemDelegate::paint(painter, option, index);
}





CommitModel::CommitModel(QObject* parent)
    :QAbstractTableModel(parent)
{

}

QVariant CommitModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_data.size() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto &item = m_data.at(index.row());

        int column = index.column();
        if(column==Flags){
            int flags =  item.flag();
            /*QString flags;
            if(item.flag(cvs::Commit::Dev)){
                flags += "Dev";
            }
            if(item.flag(cvs::Commit::Prod)){
                flags += "Prod";
            }*/
            //qDebug()<<"row:"<<index.row()<<";flags:"<<flags;
            return flags;
        }else if(column == DateTime){
            return item.time().toString("yyyy-MM-dd HH:mm");
        }else if(column == Author){
            return item.author();
        }else if(column == Content){
            return item.content();
        }
    }else if(role == Qt::BackgroundColorRole){
        int row = index.row();
        if(m_comapreRows.contains(row)){
            return QColor(255,229,143);
        }
    }else if(role == Qt::TextColorRole){
        int column = index.column();
        if(column==Flags){
            return QColor::fromRgb(0,0,0,0);
        }
    }
    return QVariant();
}

Qt::ItemFlags CommitModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    return flags;
}

QVariant CommitModel::headerData(int section, Qt::Orientation orientation,int role) const
{
    if (role == Qt::DisplayRole) {
        switch (section) {
            case Flags:
                return tr("Flags");
            case DateTime:
                return tr("Date");
            case Author:
                return tr("Author");
            case Content:
                return tr("Content");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

int CommitModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;
    return m_data.size();
}

int CommitModel::columnCount(const QModelIndex &parent) const
{
    return Max;
}

cvs::Commit CommitModel::getItem(int pos)
{
    return m_data.at(pos);
}

void CommitModel::setList(QList<cvs::Commit> data)
{
    beginResetModel();
    m_data = data;
    endResetModel();
}
void CommitModel::appendList(QList<cvs::Commit> data)
{
    int count = rowCount();
    beginInsertRows(QModelIndex(), count, count+data.size()-1);
    m_data<<data;
    endInsertRows();
}

 void CommitModel::updateFlags(QList<cvs::Commit> data)
 {
    QList<cvs::Commit>::iterator iter = m_data.begin();
    int top = -1;
    int bottom = 0;
    while(iter!=m_data.end()){
        int size = data.size();
        if(size==0){
            break;
        }
        for(int i=0;i<size;i++){
            cvs::Commit one = data.at(i);
            if((*iter).oid() == one.oid()){
                if(top==-1){
                    top = bottom;
                }
                (*iter).setFlagValue(one.flag());
                data.removeAt(i);
                goto break_data;
            }else{
                bottom+=1;
            }
        }
        break_data:

        iter++;
    }
    if(top >=0 && bottom >=0 ){
        QModelIndex leftTop = createIndex(top,Flags);
        QModelIndex rightBottom = createIndex(bottom,Flags);
        emit dataChanged(leftTop,rightBottom,QVector<int>(Qt::DisplayRole));
    }
 }



QList<cvs::Commit> CommitModel::compareRows()
{
    std::sort(m_comapreRows.begin(),m_comapreRows.end());
    QList<cvs::Commit> lists;
    int rowCount = this->rowCount();
    for(auto row:m_comapreRows){
        if(row>=0 && row<rowCount){
            lists.push_back(getItem(row));
        }
    }
    return lists;
}

int CommitModel::compareRow(int row)
{
    int size = m_comapreRows.size();
    if(size==2){
        m_comapreRows.clear();
    }
    if(m_comapreRows.contains(row)==false){
        m_comapreRows.push_back(row);
        QModelIndex leftTop = createIndex(row,Flags);
        QModelIndex rightBottom = createIndex(row,Content);
        emit dataChanged(leftTop,rightBottom,QVector<int>(Qt::BackgroundColorRole));
    }
    return m_comapreRows.size();
}

void CommitModel::clearCompareRows()
{
    QList<int>::iterator iter = m_comapreRows.begin();
    while(iter!=m_comapreRows.end()){
        int row = (*iter);
        QModelIndex leftTop = createIndex(row,Flags);
        QModelIndex rightBottom = createIndex(row,Content);
        m_comapreRows.erase(iter);
        emit dataChanged(leftTop,rightBottom,QVector<int>(Qt::BackgroundColorRole));
        iter++;
    }
}

}
