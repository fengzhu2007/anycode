#include "resource_manager_tree_item_delegate.h"
#include "resource_manager_model_item.h"
#include "resource_manager_model.h"
#include "w_toast.h"
#include <QTimer>
#include <QLineEdit>
#include <QTreeView>
#include <QModelIndex>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QDebug>
namespace ady{
class ItemEditorPrivate{
public:
    QModelIndex index;
    QAbstractItemModel *model;
};

ItemEditor::ItemEditor(QWidget* parent,QAbstractItemModel *model,const QModelIndex& index):
    QLineEdit(parent){
    d = new ItemEditorPrivate;
    d->index = index;
    d->model = model;
}

ItemEditor::~ItemEditor(){
    delete d;
}

void ItemEditor::keyPressEvent(QKeyEvent *event){
    qDebug()<<"ItemEditor keyPressEvent"<<event->key();
    if (event->key() == Qt::Key_Escape) {
        emit escapePressed(d->model,d->index);
    } else {
        QLineEdit::keyPressEvent(event);
    }
}



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

void ResourceManagerTreeItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
    if (lineEdit) {
        QString text = lineEdit->text();
        auto item = static_cast<ResourceManagerModelItem*>(index.internalPointer());
        const QString path = item->path();
        //qDebug()<<"text:"<<text;
        if(text.isEmpty()){
            if(path.isEmpty()){
                //remove index
                static_cast<ResourceManagerModel*>(model)->removeItem(item);
            }else{
                QTimer::singleShot(0, [index, editor]() {
                    auto treeView = static_cast<QTreeView*>(editor->parentWidget()->parentWidget());
                    treeView->edit(index);
                });
            }
        }else{
            //ResourceManagerModelItem::Type type = item->type();
            const QString title = item->title();
            if(title!=text){
                auto parent = item->parent();
                QFileInfo fi(parent->path(),text);
                if(fi.exists()){
                    wToast::showText(tr("'%1' already exist.").arg(text));
                    QTimer::singleShot(0, [index, editor]() {
                        auto treeView = static_cast<QTreeView*>(editor->parentWidget()->parentWidget());
                        treeView->edit(index);
                    });
                    return ;
                }
            }
            QStyledItemDelegate::setModelData(editor, model, index);
        }
    }
}

/*QWidget* ResourceManagerTreeItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const{
    ItemEditor *editor = new ItemEditor(parent,const_cast<QAbstractItemModel*>(index.model()),index);
    connect(editor, &ItemEditor::escapePressed, this, &ResourceManagerTreeItemDelegate::onEscapePressed);
    return editor;
}*/



}
