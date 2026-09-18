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

void PartObject::setPartId(const QString &id)
{
    if (m_partId != id) { m_partId = id; emit dataChanged(); }
}

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

void PartObject::setLinesAdded(int n)
{
    if (m_linesAdded != n) { m_linesAdded = n; emit dataChanged(); }
}

void PartObject::setLinesRemoved(int n)
{
    if (m_linesRemoved != n) { m_linesRemoved = n; emit dataChanged(); }
}

void PartObject::openFile()
{
    if (!m_content.isEmpty())
        emit fileOpenRequested(m_content);
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
// QmlMessageModel — signal forwarding helper
// ============================================================================

void QmlMessageModel::connectPartSignals(PartObject *p)
{
    if (!p) return;
    connect(p, &PartObject::permissionReplied, this, &QmlMessageModel::permissionReplied);
    connect(p, &PartObject::fileOpenRequested, this, &QmlMessageModel::fileOpenRequested);
}

// ============================================================================
// Server part mapping (opencode v1 part-driven rendering)
// ============================================================================

namespace {

static bool isCommandToolType(const QString &toolType)
{
    QString lt = toolType.toLower();
    return lt == "bash" || lt == "cmd" || lt == "powershell"
        || lt == "shell" || lt.contains("terminal");
}

static QString shellTypeFor(const QString &toolType, const QString &toolName)
{
    QString lt = toolType.toLower();
    if (!isCommandToolType(lt)) return QString();
    if (lt == "cmd" || toolName.contains("cmd")) return "cmd";
    if (lt == "powershell" || toolName.contains("powershell")) return "powershell";
    return "shell";
}

/** Extract the human-readable detail from tool state.input, matching the
 *  flat-signal display building in ChatService (command/filePath/path/query). */
static QString toolDetailFromInput(const QJsonObject &input)
{
    if (input.contains("command")) return input["command"].toString();
    if (input.contains("url"))     return input["url"].toString();
    if (input.contains("filePath")) return input["filePath"].toString();
    if (input.contains("path"))    return input["path"].toString();
    if (input.contains("query")) {
        QString q = input["query"].toString();
        if (q.length() > 60) q = q.left(60) + "...";
        return q;
    }
    return QString();
}

/** Build a compact input summary for grep/glob tools.
 *  Format: "tool:path include pattern" (only non-empty fields). */
static QString buildInputSummary(const QString &toolType, const QJsonObject &input)
{
    QStringList detailParts;
    if (input.contains("path"))    detailParts << input["path"].toString();
    if (input.contains("include")) detailParts << input["include"].toString();
    if (input.contains("pattern")) detailParts << input["pattern"].toString();
    if (input.contains("glob"))    detailParts << input["glob"].toString();
    if (detailParts.isEmpty()) return toolType;
    return toolType + ":" + detailParts.join(" ");
}

/** Build input summary for read tool.
 *  Format: "read:filename offset limit" (only non-empty fields). */
static QString buildReadSummary(const QJsonObject &input)
{
    QStringList parts;
    parts << "read";
    // Extract filename from path
    QString path = input["path"].toString();
    if (!path.isEmpty()) {
        int lastSlash = qMax(path.lastIndexOf('/'), path.lastIndexOf('\\'));
        QString filename = (lastSlash >= 0) ? path.mid(lastSlash + 1) : path;
        parts << filename;
    }
    if (input.contains("offset")) parts << QString::number(input["offset"].toInt());
    if (input.contains("limit"))  parts << QString::number(input["limit"].toInt());
    return parts.join(" ");
}

/** Count lines in a string (empty string = 0 lines). */
static int countLines(const QString &s)
{
    if (s.isEmpty()) return 0;
    return s.count('\n') + 1;
}

/** Part types that have a visual card; anything else (step-start,
 *  step-finish, metadata) is dropped without creating a PartObject. */
static bool isDisplayableServerPartType(const QString &t)
{
    return t == "text" || t == "reasoning" || t == "thinking" || t == "tool";
}

/** Map a raw server part onto a PartObject. Returns false for part types
 *  without visual representation (step markers, metadata, ...). */
static bool applyServerPart(PartObject *p, const QString &partType,
                            const QJsonObject &part)
{
    if (partType == "text") {
        p->setPartType("text");
        p->setContent(part["text"].toString());
        return true;
    }
    if (partType == "reasoning" || partType == "thinking") {
        p->setPartType("thinking");
        QString text = part["text"].toString();
        if (text.isEmpty()) text = part["content"].toString();
        p->setContent(text);
        return true;
    }
    if (partType == "tool") {
        QString toolType = part["tool"].toString();
        QJsonObject state = part["state"].toObject();
        QString status = state["status"].toString("pending");
        QJsonObject input = state["input"].toObject();
        QString title = part["title"].toString();
        if (title.isEmpty()) title = state["title"].toString();

        // Display name matches the flat path: "tool: <title|input detail>"
        QString detail = !title.isEmpty() ? title : toolDetailFromInput(input);
        QString display = toolType;
        if (!detail.isEmpty()) display += ": " + detail;

        // Body is the raw input detail (command text for command-line tools).
        // For grep/glob/read, use formatted summary.
        // Keep empty if input is empty — no fallback to title.
        QString body;
        QString lt = toolType.toLower();
        if (lt == "grep" || lt == "glob") {
            body = buildInputSummary(lt, input);
        } else if (lt == "read") {
            body = buildReadSummary(input);
        } else {
            body = toolDetailFromInput(input);
        }
        QString output;
        if (status == "error") {
            output = state["error"].toString();
            if (output.isEmpty()) output = state["output"].toString();
        } else if (status == "completed") {
            output = state["output"].toString();
        }

        p->setPartType("tool");
        p->setToolType(toolType.toLower());
        p->setToolName(display);
        QString callID = part["callID"].toString();
        if (callID.isEmpty()) callID = part["id"].toString();
        p->setCallID(callID);
        p->setStatus(status == "completed" ? 1 : status == "error" ? 2 : 0);
        p->setIsCommand(isCommandToolType(toolType));
        p->setShellType(shellTypeFor(toolType, display));
        // content always holds the command/input text (body).
        // output is kept separately in the output property.
        p->setContent(body);
        p->setOutput(output);

        // Calculate lines added/removed for write/edit tools
        if (lt == "write") {
            // write: content is the full new content
            p->setLinesAdded(countLines(input["content"].toString()));
            p->setLinesRemoved(0);
        } else if (lt == "edit") {
            // edit: diff between oldText and newText
            int oldLines = countLines(input["oldText"].toString());
            int newLines = countLines(input["newText"].toString());
            p->setLinesAdded(newLines);
            p->setLinesRemoved(oldLines);
        }

        return true;
    }
    return false;
}

} // namespace

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
        entry.messageId = mi.messageId;
        if (mi.type == ChatMessageBubble::Assistant && !mi.parts.isEmpty()) {
            // Part-driven history row: build parts offline from the raw
            // server parts so delegates are created with final data in place.
            entry.partsManaged = true;
            for (const auto &pv : mi.parts) {
                QJsonObject part = pv.toObject();
                QString pt = part["type"].toString();
                auto *p = new PartObject(this);
                connectPartSignals(p);
                p->setPartId(part["id"].toString());
                if (applyServerPart(p, pt, part)) {
                    QQmlEngine::setObjectOwnership(p, QQmlEngine::CppOwnership);
                    entry.parts.append(p);
                } else {
                    p->deleteLater();
                }
            }
            if (entry.parts.isEmpty()) {
                // Only step markers / metadata — fall back to flat rendering
                entry.partsManaged = false;
            }
        }
        if (!entry.partsManaged)
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
        entry.messageId = mi.messageId;
        if (mi.type == ChatMessageBubble::Assistant && !mi.parts.isEmpty()) {
            entry.partsManaged = true;
            for (const auto &pv : mi.parts) {
                QJsonObject part = pv.toObject();
                QString pt = part["type"].toString();
                auto *p = new PartObject(this);
                connectPartSignals(p);
                p->setPartId(part["id"].toString());
                if (applyServerPart(p, pt, part)) {
                    QQmlEngine::setObjectOwnership(p, QQmlEngine::CppOwnership);
                    entry.parts.append(p);
                } else {
                    p->deleteLater();
                }
            }
            if (entry.parts.isEmpty())
                entry.partsManaged = false;
        }
        if (!entry.partsManaged)
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
        entry.messageId = mi.messageId;
        if (mi.type == ChatMessageBubble::Assistant && !mi.parts.isEmpty()) {
            entry.partsManaged = true;
            for (const auto &pv : mi.parts) {
                QJsonObject part = pv.toObject();
                QString pt = part["type"].toString();
                auto *p = new PartObject(this);
                connectPartSignals(p);
                p->setPartId(part["id"].toString());
                if (applyServerPart(p, pt, part)) {
                    QQmlEngine::setObjectOwnership(p, QQmlEngine::CppOwnership);
                    entry.parts.append(p);
                } else {
                    p->deleteLater();
                }
            }
            if (entry.parts.isEmpty())
                entry.partsManaged = false;
        }
        m_messages.append(entry);
    }
    endInsertRows();

    // Build parts for the newly inserted rows and notify QML
    for (int i = first; i <= last; ++i) {
        if (!m_messages[i].partsManaged)
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
    // Part-driven rows (created from message.part.updated) may already exist
    // when the streamStarted event arrives late — reuse the unfinished
    // streaming row instead of stacking a second one.
    if (!m_messages.isEmpty() && m_messages.last().streaming) return;
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
        // Re-detect command-line
        QString lt = toolType.toLower();
        // For grep/glob/read, build formatted summary from JSON input
        if (lt == "grep" || lt == "glob" || lt == "read") {
            QJsonObject inputObj;
            QJsonDocument doc = QJsonDocument::fromJson(input.toUtf8());
            if (doc.isObject()) inputObj = doc.object();
            if (lt == "read") {
                tool.body = buildReadSummary(inputObj);
            } else {
                tool.body = buildInputSummary(lt, inputObj);
            }
        } else {
            tool.body = input;
        }
        // Calculate lines for write/edit tools
        if (lt == "write" || lt == "edit") {
            QJsonObject inputObj;
            QJsonDocument doc = QJsonDocument::fromJson(input.toUtf8());
            if (doc.isObject()) inputObj = doc.object();
            if (lt == "write") {
                tool.linesAdded = countLines(inputObj["content"].toString());
                tool.linesRemoved = 0;
            } else { // edit
                tool.linesAdded = countLines(inputObj["newText"].toString());
                tool.linesRemoved = countLines(inputObj["oldText"].toString());
            }
        }
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

    QString lt = toolType.toLower();
    // For grep/glob/read, build formatted summary from JSON input
    if (lt == "grep" || lt == "glob" || lt == "read") {
        QJsonObject inputObj;
        QJsonDocument doc = QJsonDocument::fromJson(input.toUtf8());
        if (doc.isObject()) inputObj = doc.object();
        if (lt == "read") {
            tool.body = buildReadSummary(inputObj);
        } else {
            tool.body = buildInputSummary(lt, inputObj);
        }
    } else {
        tool.body = input;
    }
    // Calculate lines for write/edit tools
    if (lt == "write" || lt == "edit") {
        QJsonObject inputObj;
        QJsonDocument doc = QJsonDocument::fromJson(input.toUtf8());
        if (doc.isObject()) inputObj = doc.object();
        if (lt == "write") {
            tool.linesAdded = countLines(inputObj["content"].toString());
            tool.linesRemoved = 0;
        } else { // edit
            tool.linesAdded = countLines(inputObj["newText"].toString());
            tool.linesRemoved = countLines(inputObj["oldText"].toString());
        }
    }

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
    // Clear streaming on ALL assistant rows, not just the last one.
    // Multiple rows may have streaming=true when upsertPart() creates
    // new rows for different messageIds during a single conversation turn.
    int cleared = 0;
    for (int row = 0; row < m_messages.size(); ++row) {
        if (m_messages[row].streaming) {
            m_messages[row].streaming = false;
            emit dataChanged(index(row), index(row), {StreamingRole});
            ++cleared;
        }
    }
    qDebug() << "[QmlMessageModel] endStreaming: cleared" << cleared
             << "of" << m_messages.size() << "rows";
}

// ---- part-driven (opencode v1) ----

void QmlMessageModel::upsertPart(const QString &messageId, const QString &partId,
                                 const QString &partType, const QJsonObject &part)
{
    // Step markers / metadata parts carry no visuals — never let them create
    // a row or a PartObject.
    if (!isDisplayableServerPartType(partType)) return;

    // Resolve the owning row: match by server messageId, else fall back to
    // the last unfinished streaming assistant row (created earlier by
    // streamStarted, or created here when the announcement arrives first).
    int row = -1;
    if (!messageId.isEmpty()) {
        for (int i = m_messages.size() - 1; i >= 0; --i) {
            if (m_messages[i].messageId == messageId) { row = i; break; }
        }
    }
    if (row < 0 && !m_messages.isEmpty()
        && m_messages.last().type == ChatMessageBubble::Assistant
        && m_messages.last().streaming) {
        row = m_messages.size() - 1;
    }

    if (row < 0) {
        // First part of a new assistant message
        row = m_messages.size();
        beginInsertRows(QModelIndex(), row, row);
        MessageEntry entry;
        entry.type = ChatMessageBubble::Assistant;
        entry.messageId = messageId;
        entry.streaming = true;
        entry.partsManaged = true;
        m_messages.append(entry);
        endInsertRows();
        emit countChanged();
    } else if (!m_messages[row].partsManaged) {
        // Adopt an existing flat-path streaming row for part-driven updates;
        // its old content/thinking parts were empty (nothing streamed yet).
        m_messages[row].partsManaged = true;
        if (m_messages[row].messageId.isEmpty())
            m_messages[row].messageId = messageId;
    }
    auto &msg = m_messages[row];

    // Locate the existing part by server part identity
    PartObject *target = nullptr;
    for (PartObject *p : msg.parts) {
        if (!partId.isEmpty() && p->partId() == partId) { target = p; break; }
    }
    if (!target) {
        target = new PartObject(this);
        connectPartSignals(target);
        QQmlEngine::setObjectOwnership(target, QQmlEngine::CppOwnership);
        target->setPartId(partId);
        msg.parts.append(target);   // append keeps server arrival order
    }

    if (applyServerPart(target, partType, part)) {
        msg.cachedPartsList.clear();
        msg.partsVersion++;
        emit dataChanged(index(row), index(row),
                         {PartsRole, PartsVersionRole, StreamingRole});
    }
}

void QmlMessageModel::appendPartDelta(const QString &messageId, const QString &partId,
                                      const QString &delta)
{
    if (delta.isEmpty()) return;

    int row = -1;
    if (!messageId.isEmpty()) {
        for (int i = m_messages.size() - 1; i >= 0; --i) {
            if (m_messages[i].messageId == messageId) { row = i; break; }
        }
    } else if (!m_messages.isEmpty() && m_messages.last().streaming) {
        row = m_messages.size() - 1;
    }
    if (row < 0) return;

    auto &msg = m_messages[row];
    for (PartObject *p : msg.parts) {
        if (p->partId() != partId) continue;
        // The server guarantees part.updated announced the part before its
        // deltas; a text/thinking part grows by appending (MarkdownBody and
        // ThinkingBlock both re-render from the full content).
        if (p->partType() == "text" || p->partType() == "thinking")
            p->setContent(p->content() + delta);
        msg.cachedPartsList.clear();
        msg.partsVersion++;
        emit dataChanged(index(row), index(row), {PartsRole, PartsVersionRole});
        return;
    }
}

bool QmlMessageModel::rowHasParts(int row) const
{
    return row >= 0 && row < m_messages.size() && !m_messages[row].parts.isEmpty();
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
            connectPartSignals(p);
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
        // Server-driven rows own their parts list; rebuilding here would
        // destroy the part identity/order established by upsertPart.
        if (msg.partsManaged) return;

        // Reuse existing PartObjects to keep QML bindings alive.
        // Layout: [thinking?] [text?] [tool0] [tool1] ...
        int idx = 0;

        // 1. Thinking part
        if (!msg.thinking.isEmpty()) {
            if (idx < msg.parts.size() && msg.parts[idx]->partType() == "thinking") {
                msg.parts[idx]->setContent(msg.thinking);
            } else {
                auto *p = new PartObject(this);
                connectPartSignals(p);
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
                connectPartSignals(p);
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
                connectPartSignals(p);
                p->setPartType("tool");
                p->setCallID(tool.callID);
                msg.parts.insert(partIdx, p);
            }

            p->setToolType(tool.toolType.toLower());
            p->setToolName(tool.toolName);
            p->setStatus(tool.status);
            p->setIsCommand(tool.isCommand);
            p->setShellType(tool.shellType);
            p->setContent(tool.body);
            p->setOutput(tool.output);
            p->setLinesAdded(tool.linesAdded);
            p->setLinesRemoved(tool.linesRemoved);
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
            connectPartSignals(p);
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
            connectPartSignals(p);
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
            connectPartSignals(p);
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
