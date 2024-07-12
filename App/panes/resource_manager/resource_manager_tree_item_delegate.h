#ifndef RESOURCEMANAGERTREEITEMDELEGATE_H
#define RESOURCEMANAGERTREEITEMDELEGATE_H

#include <QStyledItemDelegate >
#include <QPainter>

namespace ady{
class ResourceManagerTreeItemDelegate : public QStyledItemDelegate
{
public:
    ResourceManagerTreeItemDelegate(QObject* parent);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override ;
};
}

#endif // RESOURCEMANAGERTREEITEMDELEGATE_H
