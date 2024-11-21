#ifndef LIST_ITEM_DELEGATE_H
#define LIST_ITEM_DELEGATE_H

#include <QStyledItemDelegate>

namespace ady{
class ListItemDelegate : public QStyledItemDelegate
{
public:
    explicit ListItemDelegate(int height,QObject *parent = nullptr);
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
    int m_height;
};
}

#endif // LIST_ITEM_DELEGATE_H
