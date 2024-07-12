#include "commit_model.h"
#include <QColor>
#include <QApplication>
#include <QDebug>
namespace ady {




class CommitItemDelegatePrivate{
public:
    QPixmap devIcon;
    QPixmap prodIcon;
    QPixmap otherIcon;
    int boxWidth =20;
};



CommitItemDelegate::CommitItemDelegate(QObject* parent)
    :QStyledItemDelegate(parent)
{
    d = new CommitItemDelegatePrivate;
    d->devIcon.load(QString::fromUtf8(":/img/Resource/flag_dev.svg"));
    d->prodIcon.load(QString::fromUtf8(":/img/Resource/flag_prod.svg"));
    d->otherIcon.load(QString::fromUtf8(":/img/Resource/flag_other.svg"));
}

CommitItemDelegate::~CommitItemDelegate(){
    delete d;
}

void CommitItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    /*int col = index.column();
    if(col == CommitModel::Flags){
        QStyle* style =  QApplication::style();
        int value = index.data().toInt();
        if((value & cvs::Commit::Dev)==cvs::Commit::Dev){
            QRect rc(option.rect.x(),option.rect.y(),d->boxWidth,option.rect.height());
            style->drawItemPixmap(painter,rc,Qt::AlignCenter,d->devIcon);
        }
        if((value & cvs::Commit::Prod)==cvs::Commit::Prod){
            QRect rc(option.rect.x() + d->boxWidth,option.rect.y(),d->boxWidth,option.rect.height());
            style->drawItemPixmap(painter,rc,Qt::AlignCenter,d->prodIcon);
        }
        if((value & cvs::Commit::Other)==cvs::Commit::Other){
            QRect rc(option.rect.x() + d->boxWidth * 2,option.rect.y(),d->boxWidth,option.rect.height());
            style->drawItemPixmap(painter,rc,Qt::AlignCenter,d->otherIcon);
        }
    }*/
    QStyledItemDelegate::paint(painter, option, index);
}




class CommitModelPrivate{
public:
    QList<cvs::Commit> data;
    QList<int> comapreRows;

};




CommitModel::CommitModel(QObject* parent)
    :QAbstractTableModel(parent)
{
    d = new CommitModelPrivate;
}

CommitModel::~CommitModel(){
    delete d;
}

QVariant CommitModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= d->data.size() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto &item = d->data.at(index.row());

        int column = index.column();
        /*if(column==Flags){
            int flags =  item.flag();
            return flags;
        }else */if(column == DateTime){
            return item.time().toString("MM-dd HH:mm");
        }else if(column == Author){
            return item.author();
        }else if(column == Content){
            return item.content();
        }
    }else if(role == Qt::ToolTipRole){
        const auto &item = d->data.at(index.row());
        return tr("ID:%1\nDate:%2\nAuthor:%3\nMessage:%4").arg(item.oid()).arg(item.time().toString("yyyy-MM-dd HH:mm")).arg(item.author()).arg(item.content());
    }else if(role == Qt::BackgroundRole){
        int row = index.row();
        if(d->comapreRows.contains(row)){
            return QColor(255,229,143);
        }
    }/*else if(role == Qt::ForegroundRole){
        int column = index.column();
        if(column==Flags){
            return QColor::fromRgb(0,0,0,0);
        }
    }*/
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
            //case Flags:
            //    return tr("Flags");
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
    return d->data.size();
}

int CommitModel::columnCount(const QModelIndex &parent) const
{
    return Max;
}

cvs::Commit CommitModel::getItem(int pos)
{
    return d->data.at(pos);
}

void CommitModel::setList(QList<cvs::Commit> data)
{
    beginResetModel();
    d->data = data;
    endResetModel();
}

cvs::Commit CommitModel::at(int row){
    return d->data.at(row);
}

void CommitModel::setDataSource(QList<cvs::Commit> data){
    beginResetModel();
    d->data = data;
    endResetModel();
}

void CommitModel::appendList(QList<cvs::Commit> data)
{
    int count = rowCount();
    beginInsertRows(QModelIndex(), count, count+data.size()-1);
    d->data<<data;
    endInsertRows();
}

 void CommitModel::updateFlags(QList<cvs::Commit> data)
 {
    QList<cvs::Commit>::iterator iter = d->data.begin();
    int top = -1;
    int bottom = 0;
    while(iter!=d->data.end()){
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
        //QModelIndex leftTop = createIndex(top,Flags);
        //QModelIndex rightBottom = createIndex(bottom,Flags);
        //emit dataChanged(leftTop,rightBottom,QVector<int>(Qt::DisplayRole));
    }
 }



QList<cvs::Commit> CommitModel::compareRows()
{
    std::sort(d->comapreRows.begin(),d->comapreRows.end());
    QList<cvs::Commit> lists;
    int rowCount = this->rowCount();
    for(auto row:d->comapreRows){
        if(row>=0 && row<rowCount){
            lists.push_back(getItem(row));
        }
    }
    return lists;
}

int CommitModel::compareRow(int row)
{
    int size = d->comapreRows.size();
    if(size==2){
        d->comapreRows.clear();
    }
    if(d->comapreRows.contains(row)==false){
        d->comapreRows.push_back(row);
        //QModelIndex leftTop = createIndex(row,Flags);
        //QModelIndex rightBottom = createIndex(row,Content);
        //emit dataChanged(leftTop,rightBottom,QVector<int>(Qt::BackgroundRole));
    }
    return d->comapreRows.size();
}

void CommitModel::clearCompareRows()
{
    QList<int>::iterator iter = d->comapreRows.begin();
    while(iter!=d->comapreRows.end()){
        int row = (*iter);
        //QModelIndex leftTop = createIndex(row,Flags);
        //QModelIndex rightBottom = createIndex(row,Content);
        //d->comapreRows.erase(iter);
        //emit dataChanged(leftTop,rightBottom,QVector<int>(Qt::BackgroundRole));
        iter++;
    }
}

}
