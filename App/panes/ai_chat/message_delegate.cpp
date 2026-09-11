/**
 * @file message_delegate.cpp
 * @brief Custom delegate for MessageListView.
 *
 * sizeHint() uses the viewport width (from option.rect) to query the
 * ChatMessageView's heightForWidth(), ensuring the row height matches
 * the actual rendered content height.
 *
 * paint() positions the index widget at the correct geometry instead of
 * drawing via the default styled-item pipeline (which would draw text
 * that is already rendered by the widget).
 */
#include "message_delegate.h"
#include "chat_message_view.h"

#include <QPainter>
#include <QListView>

namespace ady {

MessageDelegate::MessageDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    // option.rect.width() is set by QListView to the viewport content width,
    // which is the width the index widget will actually be laid out at.
    const int w = option.rect.width();
    if (w <= 0)
        return QStyledItemDelegate::sizeHint(option, index);

    QListView *view = qobject_cast<QListView *>(const_cast<QAbstractItemView *>(
        qobject_cast<const QAbstractItemView *>(option.widget)));
    if (!view)
        return QStyledItemDelegate::sizeHint(option, index);

    QWidget *wgt = view->indexWidget(index);
        auto *msgWidget = qobject_cast<ChatMessageView *>(wgt);
    if (!msgWidget)
        return QStyledItemDelegate::sizeHint(option, index);

    // Use the widget's heightForWidth (which accounts for layout margins
    // and label wrapping) at the viewport width.
    if (msgWidget->hasHeightForWidth()) {
        int h = msgWidget->heightForWidth(w);
        return QSize(w, qMax(h, 0));
    }

    return msgWidget->sizeHint();
}

void MessageDelegate::paint(QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    QListView *view = qobject_cast<QListView *>(
        const_cast<QAbstractItemView *>(
            qobject_cast<const QAbstractItemView *>(option.widget)));

    if (view && view->indexWidget(index)) {
        // The index widget is already positioned by QListView's internal layout
        // and handles its own rendering — nothing to paint here.
        return;
    }

    // No index widget (off-screen / not yet created) — fall back to default.
    QStyledItemDelegate::paint(painter, option, index);
}

} // namespace ady
