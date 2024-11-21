#include "list_item_delegate.h"

namespace ady{

ListItemDelegate::ListItemDelegate(int height,QObject *parent)
    : QStyledItemDelegate{parent},m_height(height)
{


}


QSize ListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return {100,m_height};
}

}


