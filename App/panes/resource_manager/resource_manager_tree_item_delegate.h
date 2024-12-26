#ifndef RESOURCEMANAGERTREEITEMDELEGATE_H
#define RESOURCEMANAGERTREEITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QItemDelegate>
#include <QPainter>
#include <QLineEdit>

namespace ady{



class ResourceManagerTreeItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    ResourceManagerTreeItemDelegate(QObject* parent);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override ;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    //virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setSearchText(const QString& text);

private:
    void drawText(QPainter *painter,const QStyleOptionViewItem &option,const QRect &rect,const QModelIndex &index) const;

private:
    QString m_searchText;
};
}

#endif // RESOURCEMANAGERTREEITEMDELEGATE_H
