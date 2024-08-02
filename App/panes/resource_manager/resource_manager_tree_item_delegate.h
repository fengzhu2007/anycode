#ifndef RESOURCEMANAGERTREEITEMDELEGATE_H
#define RESOURCEMANAGERTREEITEMDELEGATE_H

#include <QStyledItemDelegate >
#include <QPainter>
#include <QLineEdit>

namespace ady{

class ItemEditorPrivate;
class ItemEditor : public QLineEdit{
    Q_OBJECT
public:
    ItemEditor(QWidget *parent,QAbstractItemModel *model,const QModelIndex& index);
    ~ItemEditor();
protected:
    virtual void keyPressEvent(QKeyEvent *event) override ;
signals:
    void escapePressed(QAbstractItemModel *model,const QModelIndex& index);
private:
    ItemEditorPrivate* d;

};



class ResourceManagerTreeItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ResourceManagerTreeItemDelegate(QObject* parent);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override ;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    //virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
}

#endif // RESOURCEMANAGERTREEITEMDELEGATE_H
