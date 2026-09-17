#include "qml_message_model.h"

#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlEngine>
#include <QElapsedTimer>

namespace ady {

// ============================================================================
// PartObject
// ============================================================================

PartObject::PartObject(QObject *parent)
    : QObject(parent)
{}

void PartObject::setPartType(const QString &t)
{
    if (m_partType != t) { m_partType = t; emit dataChanged(); }
}

void PartObject::setContent(const QString &c)
{
    if (m_content != c) { m_content = c; emit contentChanged(); }
}

void PartObject::setCallID(const QString &id)
{
    if (m_callID != id) { m_callID = id; emit dataChanged(); }
}

void PartObject::setToolType(const QString &t)
{
    if (m_toolType != t) { m_toolType = t; emit dataChanged(); }
}

void PartObject::setToolName(const QString &n)
{
    if (m_toolName != n) { m_toolName = n; emit dataChanged(); }
}

void PartObject::setStatus(int s)
{
    if (m_status != s) { m_status = s; emit dataChanged(); }
}

void PartObject::setIsCommand(bool v)
{
    if (m_isCommand != v) { m_isCommand = v; emit dataChanged(); }
}

void PartObject::setShellType(const QString &s)
{
    if (m_shellType != s) { m_shellType = s; emit dataChanged(); }
}

void PartObject::setOutput(const QString &o)
{
    if (m_output != o) { m_output = o; emit outputChanged(); }
}

void PartObject::setPermissionRequestId(const QString &id)
{
    if (m_permRequestId != id) { m_permRequestId = id; emit dataChanged(); }
}

void PartObject::replyPermission(const QString &reply)
{
    if (!m_permReply.isEmpty() || m_permRequestId.isEmpty())
        return;
    m_permReply = reply;
    emit permissionReplyChanged();
    emit permissionReplied(m_permRequestId, reply);
}

// ============================================================================
// QmlMessageModel
// ============================================================================

QmlMessageModel::QmlMessageModel(QObject *parent)
    : QAbstractListModel(parent)
{}

int QmlMessageModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_messages.size();
}

QHash<int, QByteArray> QmlMessageModel::roleNames() const
{
    QHash<int, QByteArray> r;
    r[TypeRole]           = "messageType";
    r[ContentRole]        = "content";
    r[ThinkingRole]       = "thinking";
    r[StreamingRole]      = "streaming";
    r[PartsRole]          = "parts";
    r[PartsVersionRole]   = "partsVersion";
    r[PermissionIdRole]   = "permissionId";
    r[PermissionReplyRole]= "permissionReply";
    return r;
}

QVariant QmlMessageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_messages.size())
        return {};

    const auto &msg = m_messages.at(index.row());
    switch (role) {
    case TypeRole:
        return ChatMessageBubble::roleOf(msg.type);
    case ContentRole:
        return msg.content;
    case ThinkingRole:
        return msg.thinking;
    case StreamingRole:
        return msg.streaming;
    case PartsRole: {
        // Return cached list to avoid Repeater rebuilding all delegates on every access
        if (msg.cachedPartsList.isEmpty() && !msg.parts.isEmpty()) {
            for (auto *p : msg.parts)
                msg.cachedPartsList.append(QVariant::fromValue(static_cast<QObject*>(p)));
        }
        return msg.cachedPartsList;
    }
    case PartsVersionRole:
        return msg.partsVersion;
    case PermissionIdRole:
        return msg.type == ChatMessageBubble::Permission
                   ? QJsonDocument::fromJson(msg.content.toUtf8())
                         .object().value("requestId").toString()
                   : QString();
    case PermissionReplyRole:
        return QString();
    }
    return {};
}

// ---- message management ----

void QmlMessageModel::addMessage(ChatMessageBubble::Type type, const QString &content)
{
    int row = m_messages.size();
    beginInsertRows(QModelIndex(), row, row);
    MessageEntry entry;
    entry.type = type;
    entry.content = content;
    m_messages.append(entry);
    endInsertRows();

    rebuildParts(row);
    // Notify QML that parts/thinking/content changed after rebuild
    emit dataChanged(index(row), index(row), {TypeRole, ContentRole, ThinkingRole, StreamingRole, PartsRole, PartsVersionRole});
    emit countChanged();
}

void QmlMessageModel::insertMessage(int row, ChatMessageBubble::Type type, const QString &content)
{
    if (row < 0 || row > m_messages.size()) return;
    beginInsertRows(QModelIndex(), row, row);
    MessageEntry entry;
    entry.type = type;
    entry.content = content;
    m_messages.insert(row, entry);
    endInsertRows();

    // Rebuild call-ID map (rows shifted)
    m_callIdMap.clear();
    for (int i = 0; i < m_messages.size(); ++i) {
        for (int t = 0; t < m_messages[i].toolCalls.size(); ++t)
            m_callIdMap[m_messages[i].toolCalls[t].callID] = {i, t};
        rebuildParts(i);
        emit dataChanged(index(i), index(i), {TypeRole, ContentRole, ThinkingRole, StreamingRole, PartsRole, PartsVersionRole});
    }
    emit countChanged();
}

void QmlMessageModel::removeMessage(int row)
{
    if (row < 0 || row >= m_messages.size()) return;

    // Remove call-ID entries for this row
    auto it = m_callIdMap.begin();
    while (it != m_callIdMap.end()) {
        if (it.value().first == row)
            it = m_callIdMap.erase(it);
        else {
            if (it.value().first > row)
                it.value().first--;
            ++it;
        }
    }

    clearParts(m_messages[row].parts);
    beginRemoveRows(QModelIndex(), row, row);
    m_messages.removeAt(row);
    endRemoveRows();
    emit countChanged();
}

void QmlMessageModel::clearMessages()
{
    if (m_messages.isEmpty()) return;
    for (auto &m : m_messages) clearParts(m.parts);
    m_callIdMap.clear();
    beginResetModel();
    m_messages.clear();
    endResetModel();
    emit countChanged();
}

void QmlMessageModel::resetMessages(const QList<MsgInput> &messages)
{
    QElapsedTimer totalTimer;
    totalTimer.start();

    // Clean up old PartObjects
    for (auto &m : m_messages) clearParts(m.parts);
    m_callIdMap.clear();

    // Build entries WITH parts fully offline, BEFORE the model reset. This
    // way the delegates created by endResetModel() read final data right
    // away: no empty-height delegate pass, no per-row dataChanged storm,
    // and no double instantiate/destroy cycle per message.
    QList<MessageEntry> newEntries;
    newEntries.reserve(messages.size());
    for (const auto &mi : messages) {
        MessageEntry entry;
        entry.type = mi.type;
        entry.content = mi.content;
        entry.thinking = mi.thinking;
        buildEntryParts(entry);
        newEntries.append(entry);
    }

    // Swap in and reset model in one shot
    beginResetModel();
    m_messages = newEntries;
    endResetModel();

    // Rebuild call-ID map for any entries carrying tool calls
    for (int i = 0; i < m_messages.size(); ++i)
        for (int t = 0; t < m_messages[i].toolCalls.size(); ++t)
            m_callIdMap[m_messages[i].toolCalls[t].callID] = {i, t};

    emit countChanged();
    qDebug() << "[QmlMessageModel] resetMessages: count=" << m_messages.size()
             << "total=" << totalTimer.elapsed() << "ms";
}

void QmlMessageModel::prependMessages(const QList<MsgInput> &messages)
{
    if (messages.isEmpty()) return;

    // Build the older entries offline first — existing rows and their
    // delegates are left untouched, so nothing visible is destroyed.
    QList<MessageEntry> newEntries;
    newEntries.reserve(messages.size());
    for (const auto &mi : messages) {
        MessageEntry entry;
        entry.type = mi.type;
        entry.content = mi.content;
        entry.thinking = mi.thinking;
        buildEntryParts(entry);
        newEntries.append(entry);
    }

    // Shift call-ID map rows (all existing rows move down by messages.size())
    const int shift = messages.size();
    for (auto it = m_callIdMap.begin(); it != m_callIdMap.end(); ++it)
        it.value().first += shift;

    // Let the QML view suppress auto-scroll and prepare its re-anchor
    emit aboutToPrependMessages(shift);

    beginInsertRows(QModelIndex(), 0, shift - 1);
    QList<MessageEntry> combined;
    combined.reserve(shift + m_messages.size());
    combined.append(newEntries);
    combined.append(m_messages);
    m_messages = combined;
    endInsertRows();

    // Map call IDs of the freshly prepended rows
    for (int i = 0; i < shift && i < m_messages.size(); ++i)
        for (int t = 0; t < m_messages[i].toolCalls.size(); ++t)
            m_callIdMap[m_messages[i].toolCalls[t].callID] = {i, t};

    emit countChanged();
}

void QmlMessageModel::appendMessages(const QList<MsgInput> &messages)
{
    if (messages.isEmpty()) return;

    int first = m_messages.size();
    int last  = first + messages.size() - 1;

    beginInsertRows(QModelIndex(), first, last);
    for (const auto &mi : messages) {
        MessageEntry entry;
        entry.type = mi.type;
        entry.content = mi.content;
        entry.thinking = mi.thinking;
        m_messages.append(entry);
    }
    endInsertRows();

    // Build parts for the newly inserted rows and notify QML
    for (int i = first; i <= last; ++i) {
        rebuildParts(i);
        emit dataChanged(index(i), index(i), {TypeRole, ContentRole, ThinkingRole, StreamingRole, PartsRole, PartsVersionRole});
    }

    emit countChanged();
}

ChatMessageBubble::Type QmlMessageModel::messageType(int row) const
{
    return (row >= 0 && row < m_messages.size()) ? m_messages[row].type : ChatMessageBubble::System;
}

QString QmlMessageModel::messageContent(int row) const
{
    return (row >= 0 && row < m_messages.size()) ? m_messages[row].content : QString();
}

QString QmlMessageModel::messageThinking(int row) const
{
    return (row >= 0 && row < m_messages.size()) ? m_messages[row].thinking : QString();
}

// ---- streaming ----

void QmlMessageModel::beginStreaming()
{
    int row = m_messages.size();
    beginInsertRows(QModelIndex(), row, row);
    MessageEntry entry;
    entry.type = ChatMessageBubble::Assistant;
    entry.streaming = true;
    m_messages.append(entry);
    endInsertRows();
    emit countChanged();
}

void QmlMessageModel::appendStreamingText(const QString &delta)
{
    if (m_messages.isEmpty()) return;
    int row = m_messages.size() - 1;
    m_messages[row].content.append(delta);
    rebuildParts(row);
    emit dataChanged(index(row), index(row), {ContentRole, PartsRole, PartsVersionRole});
}

void QmlMessageModel::appendStreamingThinking(const QString &content)
{
    if (m_messages.isEmpty()) return;
    int row = m_messages.size() - 1;
    m_messages[row].thinking = content.trimmed();
    rebuildParts(row);
    emit dataChanged(index(row), index(row), {ThinkingRole, PartsRole, PartsVersionRole});
}

void QmlMessageModel::appendToolCall(const QString &callID, const QString &toolType,
                                      const QString &toolName, const QString &input)
{
    if (m_messages.isEmpty()) return;
    int row = m_messages.size() - 1;
    auto &msg = m_messages[row];

    // Check if callID already exists (update in place).
    // Only merge when the callID belongs to the CURRENT streaming row. A map hit
    // pointing at an older row means the callID was recycled (empty callID fell
    // back to toolName, or an SSE replay) — indexing another row's toolCalls
    // here would go out of bounds and corrupt the heap.
    if (m_callIdMap.contains(callID) && m_callIdMap[callID].first == row) {
        auto &ref = m_callIdMap[callID];
        if (ref.second < 0 || ref.second >= msg.toolCalls.size()) return;
        auto &tool = msg.toolCalls[ref.second];
        tool.toolType = toolType;
        tool.toolName = toolName;
        tool.body = input;
        // Re-detect command-line
        QString lt = toolType.toLower();
        tool.isCommand = (lt == "bash" || lt == "cmd" || lt == "powershell"
                          || lt == "shell" || lt.contains("terminal"));
        if (tool.isCommand) {
            if (lt == "cmd" || toolName.contains("cmd"))
                tool.shellType = "cmd";
            else if (lt == "powershell" || toolName.contains("powershell"))
                tool.shellType = "powershell";
            else
                tool.shellType = "shell";
        }
        rebuildParts(row);
        emit dataChanged(index(row), index(row), {PartsRole, PartsVersionRole});
        return;
    }

    MessageEntry::ToolRef tool;
    tool.callID = callID;
    tool.toolType = toolType;
    tool.toolName = toolName;
    tool.status = 0; // Processing
    tool.body = input;

    QString lt = toolType.toLower();
    tool.isCommand = (lt == "bash" || lt == "cmd" || lt == "powershell"
                      || lt == "shell" || lt.contains("terminal"));
    if (tool.isCommand) {
        if (lt == "cmd" || toolName.contains("cmd"))
            tool.shellType = "cmd";
        else if (lt == "powershell" || toolName.contains("powershell"))
            tool.shellType = "powershell";
        else
            tool.shellType = "shell";
    }

    int toolIdx = msg.toolCalls.size();
    msg.toolCalls.append(tool);
    m_callIdMap[callID] = {row, toolIdx};

    rebuildParts(row);
    emit dataChanged(index(row), index(row), {PartsRole, PartsVersionRole});
}

void QmlMessageModel::updateToolCallStatus(const QString &callID, int status, const QString &output)
{
    if (!m_callIdMap.contains(callID)) return;
    const auto ref = m_callIdMap[callID];
    // Defensive bounds check — a stale or recycled callID must never index
    // another row's tool list (out-of-bounds write corrupts the heap).
    if (ref.first < 0 || ref.first >= m_messages.size()) return;
    auto &entry = m_messages[ref.first];
    if (ref.second < 0 || ref.second >= entry.toolCalls.size()) return;
    auto &tool = entry.toolCalls[ref.second];
    tool.status = status;
    if (!output.isEmpty())
        tool.output = output;

    rebuildParts(ref.first);
    emit dataChanged(index(ref.first), index(ref.first), {PartsRole, PartsVersionRole});
}

void QmlMessageModel::endStreaming()
{
    if (m_messages.isEmpty()) return;
    int row = m_messages.size() - 1;
    m_messages[row].streaming = false;
    emit dataChanged(index(row), index(row), {StreamingRole});
}

void QmlMessageModel::scrollToBottom()
{
    emit scrollToBottomRequested();
}

void QmlMessageModel::requestLoadMore()
{
    qDebug() << "[QmlMessageModel] requestLoadMore called, emitting signal";
    emit loadMoreRequested();
}

// ---- private ----

void QmlMessageModel::clearParts(QList<PartObject*> &parts)
{
    for (auto *p : parts) p->deleteLater();
    parts.clear();
}

void QmlMessageModel::rebuildParts(int row)
{
    if (row < 0 || row >= m_messages.size()) return;
    auto &msg = m_messages[row];

    // Invalidate cached QVariantList — will be rebuilt lazily on next data() access
    msg.cachedPartsList.clear();
    msg.partsVersion++;

    // Ensure QML ownership for any new parts
    for (auto *p : msg.parts)
        QQmlEngine::setObjectOwnership(p, QQmlEngine::CppOwnership);

    buildEntryParts(msg);
}

void QmlMessageModel::buildEntryParts(MessageEntry &msg)
{
    switch (msg.type) {

    case ChatMessageBubble::User: {
        // Single text part — reuse existing
        if (msg.parts.isEmpty()) {
            auto *p = new PartObject(this);
            p->setPartType("text");
            p->setContent(msg.content);
            msg.parts.append(p);
        } else if (msg.parts.size() == 1 && msg.parts[0]->partType() == "text") {
            msg.parts[0]->setContent(msg.content);
        } else {
            // Structural change: rebuild
            auto *p = msg.parts.takeFirst();
            clearParts(msg.parts);
            p->setPartType("text");
            p->setContent(msg.content);
            msg.parts.append(p);
        }
        break;
    }

    case ChatMessageBubble::Assistant: {
        // Reuse existing PartObjects to keep QML bindings alive.
        // Layout: [thinking?] [text?] [tool0] [tool1] ...
        int idx = 0;

        // 1. Thinking part
        if (!msg.thinking.isEmpty()) {
            if (idx < msg.parts.size() && msg.parts[idx]->partType() == "thinking") {
                msg.parts[idx]->setContent(msg.thinking);
            } else {
                auto *p = new PartObject(this);
                p->setPartType("thinking");
                p->setContent(msg.thinking);
                msg.parts.insert(idx, p);
            }
            idx++;
        } else {
            // Remove thinking part if it was there
            if (idx < msg.parts.size() && msg.parts[idx]->partType() == "thinking") {
                auto *p = msg.parts.takeAt(idx);
                p->deleteLater();
            }
        }

        // 2. Text part (strip workspace context prefix)
        QString displayText = msg.content;
        static QRegularExpression wsContextRe(
            QStringLiteral("^\\[Current working director(?:y|ies):.*?\\]\\n?"),
            QRegularExpression::DotMatchesEverythingOption);
        displayText.remove(wsContextRe);

        bool needTextPart = !displayText.isEmpty();
        if (needTextPart) {
            if (idx < msg.parts.size() && msg.parts[idx]->partType() == "text") {
                msg.parts[idx]->setContent(displayText);
            } else {
                auto *p = new PartObject(this);
                p->setPartType("text");
                p->setContent(displayText);
                msg.parts.insert(idx, p);
            }
            idx++;
        } else {
            // Remove text part if content became empty
            if (idx < msg.parts.size() && msg.parts[idx]->partType() == "text") {
                auto *p = msg.parts.takeAt(idx);
                p->deleteLater();
            }
        }

        // 3. Tool call parts — reuse by callID
        int toolStart = idx;
        for (int t = 0; t < msg.toolCalls.size(); ++t) {
            const auto &tool = msg.toolCalls[t];
            int partIdx = toolStart + t;

            PartObject *p = nullptr;
            if (partIdx < msg.parts.size() && msg.parts[partIdx]->partType() == "tool"
                && msg.parts[partIdx]->callID() == tool.callID) {
                // Same position, same callID — update in place
                p = msg.parts[partIdx];
            } else {
                // Need to insert a new tool part
                p = new PartObject(this);
                p->setPartType("tool");
                p->setCallID(tool.callID);
                msg.parts.insert(partIdx, p);
            }

            p->setToolType(tool.toolType.toLower());
            p->setToolName(tool.toolName);
            p->setStatus(tool.status);
            p->setIsCommand(tool.isCommand);
            p->setShellType(tool.shellType);
            p->setContent(tool.status == 0 ? tool.body
                            : (tool.output.isEmpty() ? tool.body : tool.output));
        }

        // Remove extra tool parts beyond what's needed
        int expectedEnd = toolStart + msg.toolCalls.size();
        while (msg.parts.size() > expectedEnd) {
            auto *p = msg.parts.takeAt(expectedEnd);
            p->deleteLater();
        }
        break;
    }

    case ChatMessageBubble::Event: {
        if (msg.parts.isEmpty()) {
            auto *p = new PartObject(this);
            p->setPartType("text");
            p->setContent(msg.content);
            msg.parts.append(p);
        } else {
            msg.parts[0]->setContent(msg.content);
        }
        break;
    }

    case ChatMessageBubble::Error:
    case ChatMessageBubble::System: {
        if (msg.parts.isEmpty()) {
            auto *p = new PartObject(this);
            p->setPartType("text");
            p->setContent(msg.content);
            msg.parts.append(p);
        } else {
            msg.parts[0]->setContent(msg.content);
        }
        break;
    }

    case ChatMessageBubble::Permission: {
        if (msg.parts.isEmpty() || msg.parts[0]->partType() != "permission") {
            clearParts(msg.parts);
            auto *p = new PartObject(this);
            p->setPartType("permission");
            p->setContent(msg.content);
            msg.parts.append(p);
        } else {
            msg.parts[0]->setContent(msg.content);
        }

        QJsonDocument doc = QJsonDocument::fromJson(msg.content.toUtf8());
        QJsonObject obj = doc.object();
        msg.parts[0]->setPermissionRequestId(obj["requestId"].toString());
        break;
    }

    } // switch
}

} // namespace ady
