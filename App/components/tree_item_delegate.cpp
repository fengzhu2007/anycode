#include "tree_item_delegate.h"


namespace ady{


TreeItemDelegate::TreeItemDelegate(QObject *parent)
    : QItemDelegate{parent}
{}



void TreeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const  {
    QStyleOptionViewItem opt = option;
    painter->save();
    QRect checkRect;
    QRect pixmapRect{0,0,16,16};
    QRect textRect;
    doLayout(option, &checkRect, &pixmapRect, &textRect, false);

    //draw background
    if (opt.state & QStyle::State_Selected) {

        if(opt.state & QStyle::State_HasFocus){
            painter->fillRect(QRect(0,opt.rect.y(),opt.rect.width() + opt.rect.x(),opt.rect.height()),QColor("#bb007acc"));
            opt.palette.setColor(QPalette::Text, Qt::white);
        }else{
            painter->fillRect(QRect(0,opt.rect.y(),opt.rect.width() + opt.rect.x(),opt.rect.height()),QColor("#bbcccedb"));
        }
        opt.state &= ~QStyle::State_HasFocus;
        opt.state &= ~QStyle::State_Selected;
    }else if (opt.state & QStyle::State_MouseOver) {
        opt.state &= ~QStyle::State_MouseOver;
        painter->fillRect(QRect(0,opt.rect.y(),opt.rect.width() + opt.rect.x(),opt.rect.height()),QColor("#99c9DCF5"));
    }
    //draw icon
    auto icon = index.data(Qt::DecorationRole).value<QIcon>();
    if(!icon.isNull())
        QItemDelegate::drawDecoration(painter,opt,pixmapRect,icon.pixmap(pixmapRect.size()));

    auto text = index.data(Qt::DisplayRole).value<QString>();

    QItemDelegate::drawDisplay(painter, opt, textRect, text);

    painter->restore();

}

QSize TreeItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const{
    return {0,22};
}

}
