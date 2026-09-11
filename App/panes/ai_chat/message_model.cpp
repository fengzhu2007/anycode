/**
 * @file message_model.cpp
 * @brief MessageModel implementation — lightweight data storage for chat messages.
 */
#include "message_model.h"

namespace ady {

MessageModel::MessageModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int MessageModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_messages.size();
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_messages.size())
        return {};

    const auto &msg = m_messages.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
        return msg.content;
    case Qt::UserRole:
        return static_cast<int>(msg.type);
    default:
        return {};
    }
}

void MessageModel::addMessage(ChatMessageView::Type type, const QString &content)
{
    int row = m_messages.size();
    beginInsertRows(QModelIndex(), row, row);
    m_messages.append({type, content});
    endInsertRows();
}

void MessageModel::insertMessage(int row, ChatMessageView::Type type, const QString &content)
{
    if (row < 0 || row > m_messages.size())
        row = m_messages.size();
    beginInsertRows(QModelIndex(), row, row);
    m_messages.insert(row, {type, content});
    endInsertRows();
}

void MessageModel::removeMessage(int row)
{
    if (row < 0 || row >= m_messages.size())
        return;
    beginRemoveRows(QModelIndex(), row, row);
    m_messages.removeAt(row);
    endRemoveRows();
}

void MessageModel::clearMessages()
{
    if (m_messages.isEmpty())
        return;
    beginResetModel();
    m_messages.clear();
    endResetModel();
}

MessageData MessageModel::messageAt(int row) const
{
    if (row >= 0 && row < m_messages.size())
        return m_messages.at(row);
    return {};
}

ChatMessageView::Type MessageModel::messageType(int row) const
{
    if (row >= 0 && row < m_messages.size())
        return m_messages.at(row).type;
    return ChatMessageView::Assistant;
}

QString MessageModel::messageContent(int row) const
{
    if (row >= 0 && row < m_messages.size())
        return m_messages.at(row).content;
    return {};
}

void MessageModel::notifyDataChanged(int row)
{
    if (row >= 0 && row < m_messages.size()) {
        QModelIndex idx = index(row);
        emit dataChanged(idx, idx);
    }
}

} // namespace ady
