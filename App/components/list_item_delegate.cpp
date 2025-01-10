#include "list_item_delegate.h"

namespace ady{

ListItemDelegate::ListItemDelegate(int height,QObject *parent)
    : QStyledItemDelegate{parent},m_height(height)
{


}

ListItemDelegate::ListItemDelegate(QObject *parent)
    : QStyledItemDelegate{parent},m_height(24)
{


}




QSize ListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return {0,m_height};
}

}


