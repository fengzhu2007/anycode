#include "resource_manager_tree_item_delegate.h"

#include <QDebug>
namespace ady{

ResourceManagerTreeItemDelegate::ResourceManagerTreeItemDelegate(QObject* parent)
    :QStyledItemDelegate(parent)
{

}

void ResourceManagerTreeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const  {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
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
    QStyledItemDelegate::paint(painter, opt, index);
}

}
