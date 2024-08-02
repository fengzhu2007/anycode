#include "commit_model.h"
#include <QColor>
#include <QApplication>
#include <QPainter>
#include <QTreeView>
#include <QDebug>
namespace ady {




class CommitItemDelegatePrivate{
public:
    QColor dev;
    QColor prod;
    QColor other;
};



CommitItemDelegate::CommitItemDelegate(QObject* parent)
    :QStyledItemDelegate(parent)
{
    d = new CommitItemDelegatePrivate;
    d->dev = QColor("#fa8c16");
    d->prod = QColor("#52c41a");
    d->other = QColor("#1890ff");

}

CommitItemDelegate::~CommitItemDelegate(){
    delete d;
}

void CommitItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    int col = index.column();
    if(col==CommitModel::DateTime){
        int row = index.row();
        auto model = static_cast<CommitModel*>(static_cast<QTreeView*>(parent())->model());
        auto one = model->at(row);
        const int value = one.flag();
        int start = 0;
        int x = option.rect.x() + 1;
        int y = option.rect.y() + 1 ;
        int w = (option.rect.height() - 2) / 3;
        int h = w;
        int num = 0;
        if((value & cvs::Commit::Dev)==cvs::Commit::Dev){
            num += 1;
        }
        if((value & cvs::Commit::Prod)==cvs::Commit::Prod){
            num += 1;
        }
        if((value & cvs::Commit::Other)==cvs::Commit::Other){
            num += 1;
        }
        opt.rect.setX(x + w + 1);
        if(num>0){
            y += (3 - num) * h/2;
            painter->setRenderHint(QPainter::Antialiasing);
            if((value & cvs::Commit::Dev)==cvs::Commit::Dev){
                painter->setBrush(d->dev);
                painter->setPen(d->dev);
                painter->drawEllipse(x,y + start,w,h);
                start += h;
            }
            if((value & cvs::Commit::Prod)==cvs::Commit::Prod){
                painter->setBrush(d->prod);
                painter->setPen(d->prod);
                painter->drawEllipse(x,y + start,w,h);
                start += h;
            }
            if((value & cvs::Commit::Other)==cvs::Commit::Other){
                painter->setBrush(d->other);
                painter->setPen(d->other);
                painter->drawEllipse(x,y + start,w,h);
                start += h;
            }
        }
    }
    QStyledItemDelegate::paint(painter, opt, index);
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
        return tr("Commit ID:%1\nDate:%2\nAuthor:%3\nMessage:%4").arg(item.oid()).arg(item.time().toString("yyyy-MM-dd HH:mm")).arg(item.author()).arg(item.content());
    }else if(role == Qt::BackgroundRole){
        int row = index.row();
        if(d->comapreRows.contains(row)){
            return QColor(255,229,143);
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

cvs::Commit CommitModel::at(int row) const{
    return d->data.at(row);
}

void CommitModel::setDataSource(QList<cvs::Commit> data){
    beginResetModel();
    d->data = data;
    endResetModel();
}

void CommitModel::appendList(QList<cvs::Commit> data)
{
    if(data.size()==0){
        return ;
    }
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

 void CommitModel::clearAllFlags(){
    auto iter = d->data.begin();
    while(iter!=d->data.end()){
        (*iter).setFlagValue(0);
        iter++;
    }
 }



}
