/**
 * @file message_delegate.h
 * @brief Custom delegate for MessageListView to correctly calculate
 *        variable row heights based on viewport width.
 *
 * Without this delegate, QListView's default QStyledItemDelegate does not
 * query the index widget's heightForWidth(), causing row heights to be
 * too small and message content to overlap.
 */
#ifndef MESSAGE_DELEGATE_H
#define MESSAGE_DELEGATE_H

#include <QStyledItemDelegate>

namespace ady {

class MessageDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit MessageDelegate(QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

} // namespace ady

#endif // MESSAGE_DELEGATE_H
