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
#include <QApplication>
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
    //qDebug()<<"ItemEditor keyPressEvent"<<event->key();
    if (event->key() == Qt::Key_Escape) {
        emit escapePressed(d->model,d->index);
    } else {
        QLineEdit::keyPressEvent(event);
    }
}



ResourceManagerTreeItemDelegate::ResourceManagerTreeItemDelegate(QObject* parent)
    :QItemDelegate(parent)
{

}

void ResourceManagerTreeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const  {
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
    //qDebug()<<checkRect<<pixmapRect<<textRect;
    if(!icon.isNull())
        QItemDelegate::drawDecoration(painter,opt,pixmapRect,icon.pixmap(pixmapRect.size()));

    this->drawText(painter,opt,textRect,index);

    painter->restore();

    //initStyleOption(&opt, index);
    /*if (opt.state & QStyle::State_Selected) {

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
    auto item = static_cast<ResourceManagerModelItem*>(index.internalPointer());
    if(this->m_searchText.isEmpty()==false && item->title().contains(this->m_searchText)){
        auto type = item->type();
        if(type==ResourceManagerModelItem::File || type==ResourceManagerModelItem::Folder){
            //this->drawMatchedText(painter,opt)
            drawMatchedText(painter, opt, opt.rect, index);
        }
        //painter->fillRect(QRect(0,opt.rect.y(),opt.rect.width() + opt.rect.x(),opt.rect.height()),QColor("#D0F7FF"));//208,247,255
    }
    QItemDelegate::paint(painter, opt, index);*/
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
            QItemDelegate::setModelData(editor, model, index);
        }
    }
}

QSize ResourceManagerTreeItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {

    return {0,22};
}

void ResourceManagerTreeItemDelegate::drawText(QPainter *painter,const QStyleOptionViewItem &option,const QRect &rect,const QModelIndex &index) const{
    //bool isSelected = option.state & QStyle::State_Selected;

    const auto item = static_cast<ResourceManagerModelItem*>(index.internalPointer());
    const auto text = item->title();

    const auto type = item->type();
    if(this->m_searchText.isEmpty() || (type!=ResourceManagerModelItem::File && type!=ResourceManagerModelItem::Folder) || text.contains(this->m_searchText)==false){
        QItemDelegate::drawDisplay(painter, option, rect, text);
    }else{
        const int searchTermLength = this->m_searchText.length();
        const int searchTermStart = text.indexOf(this->m_searchText);

        // clip searchTermLength to end of line

        const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
        const QString textBefore = text.left(searchTermStart);
        const QString textHighlight = text.mid(searchTermStart, searchTermLength);
        const QString textAfter = text.mid(searchTermStart + searchTermLength);
        int searchTermStartPixels = option.fontMetrics.horizontalAdvance(textBefore);
        int searchTermLengthPixels = option.fontMetrics.horizontalAdvance(textHighlight);



        QRect beforeHighlightRect(rect);
        beforeHighlightRect.setRight(beforeHighlightRect.left() + searchTermStartPixels);

        QRect resultHighlightRect(rect);
        resultHighlightRect.setLeft(beforeHighlightRect.right());
        resultHighlightRect.setRight(resultHighlightRect.left() + searchTermLengthPixels);

        QRect afterHighlightRect(rect);
        afterHighlightRect.setLeft(resultHighlightRect.right());

        QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                      ? QPalette::Normal : QPalette::Disabled;
        if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
            cg = QPalette::Inactive;
        QStyleOptionViewItem baseOption = option;
        baseOption.state &= ~QStyle::State_Selected;

        painter->fillRect(resultHighlightRect.adjusted(textMargin, 0, textMargin - 1, 0),QBrush(Qt::lightGray));

        // Text before the highlighting
        QStyleOptionViewItem noHighlightOpt = baseOption;
        noHighlightOpt.rect = beforeHighlightRect;
        noHighlightOpt.textElideMode = Qt::ElideNone;
        //if (isSelected)
        //noHighlightOpt.palette.setColor(QPalette::Text, noHighlightOpt.palette.color(cg, QPalette::HighlightedText));
        QItemDelegate::drawDisplay(painter, noHighlightOpt, beforeHighlightRect, textBefore);

        // Highlight text
        QStyleOptionViewItem highlightOpt = noHighlightOpt;

        highlightOpt.palette.setColor(QPalette::Text, Qt::black);
        QItemDelegate::drawDisplay(painter, highlightOpt, resultHighlightRect, textHighlight);

        // Text after the Highlight
        noHighlightOpt.rect = afterHighlightRect;
        QItemDelegate::drawDisplay(painter, noHighlightOpt, afterHighlightRect, textAfter);

    }


}

void ResourceManagerTreeItemDelegate::setSearchText(const QString& text){
    this->m_searchText = text;
}




}
