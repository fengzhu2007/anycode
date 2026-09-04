/**
 * @file message_model.h
 * @brief Lightweight data model for virtualized chat message list.
 *
 * Stores only message data (type + content), no widget references.
 * Widgets are managed by MessageListView for visible rows only.
 */
#ifndef MESSAGE_MODEL_H
#define MESSAGE_MODEL_H

#include <QAbstractListModel>
#include "chat_message_widget.h"

namespace ady {

struct MessageData {
    ChatMessageWidget::Type type = ChatMessageWidget::Assistant;
    QString content;
};

/**
 * MessageModel - data-only model for chat messages.
 *
 * Each row stores a lightweight MessageData struct.
 * The actual ChatMessageWidget instances are created/destroyed
 * by MessageListView as rows enter/leave the visible viewport.
 */
class MessageModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit MessageModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    /** Append a new message at the end. */
    void addMessage(ChatMessageWidget::Type type, const QString &content);

    /** Insert a message at a specific row. */
    void insertMessage(int row, ChatMessageWidget::Type type, const QString &content);

    /** Remove a single row. */
    void removeMessage(int row);

    /** Remove all messages and destroy all index widgets. */
    void clearMessages();

    /** Get message data by row. */
    MessageData messageAt(int row) const;

    /** Get message type by row. */
    ChatMessageWidget::Type messageType(int row) const;

    /** Get raw content by row. */
    QString messageContent(int row) const;

    /** Notify the view that a row's data has changed (triggers layout recalc). */
    void notifyDataChanged(int row);

    /** Total message count (convenience). */
    int messageCount() const { return m_messages.size(); }

private:
    QList<MessageData> m_messages;
};

} // namespace ady

#endif // MESSAGE_MODEL_H
