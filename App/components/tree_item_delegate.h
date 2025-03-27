#ifndef TREE_ITEM_DELEGATE_H
#define TREE_ITEM_DELEGATE_H

#include <QItemDelegate>
#include <QPainter>
#include "global.h"
namespace ady{
class ANYENGINE_EXPORT TreeItemDelegate : public QItemDelegate
{
public:
    explicit TreeItemDelegate(QObject *parent = nullptr);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override ;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

}

#endif // TREE_ITEM_DELEGATE_H
