#ifndef QML_MESSAGE_MODEL_H
#define QML_MESSAGE_MODEL_H

/**
 * @file qml_message_model.h
 * @brief QML-compatible message model with part-level decomposition.
 *
 * PartObject  — per-part QObject with NOTIFY signals for real-time QML binding.
 * QmlMessageModel — QAbstractListModel that parses each message into an ordered
 *                   list of PartObjects (thinking / text / tool-call / permission).
 *
 * Designed as a drop-in replacement for MessageModel + MessageListView +
 * ChatMessageBubble when embedded via QQuickWidget.  The old classes are
 * kept intact; switching is controlled by the caller (SessionPageWidget).
 */

#include <QAbstractListModel>
#include <QObject>
#include <QHash>
#include <QList>
#include <QJsonArray>
#include <QQmlEngine>
#include "chat_message_bubble.h"

namespace ady {

// ---------------------------------------------------------------------------
// PartObject — a single content block exposed to QML with NOTIFY signals
// ---------------------------------------------------------------------------
class PartObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString partId READ partId NOTIFY dataChanged)
    Q_PROPERTY(QString partType READ partType NOTIFY dataChanged)
    Q_PROPERTY(QString content READ content WRITE setContent NOTIFY contentChanged)
    Q_PROPERTY(QString callID  READ callID  NOTIFY dataChanged)
    Q_PROPERTY(QString toolType READ toolType NOTIFY dataChanged)
    Q_PROPERTY(QString toolName READ toolName NOTIFY dataChanged)
    Q_PROPERTY(int    status    READ status   NOTIFY dataChanged)
    Q_PROPERTY(bool   isCommand READ isCommand NOTIFY dataChanged)
    Q_PROPERTY(QString shellType READ shellType NOTIFY dataChanged)
    Q_PROPERTY(QString output   READ output  WRITE setOutput NOTIFY outputChanged)
    Q_PROPERTY(int linesAdded READ linesAdded WRITE setLinesAdded NOTIFY dataChanged)
    Q_PROPERTY(int linesRemoved READ linesRemoved WRITE setLinesRemoved NOTIFY dataChanged)
    Q_PROPERTY(QString permissionRequestId READ permissionRequestId NOTIFY dataChanged)
    Q_PROPERTY(QString permissionReply     READ permissionReply     NOTIFY permissionReplyChanged)
    Q_PROPERTY(QString todoListId READ todoListId NOTIFY dataChanged)
    Q_PROPERTY(QString todoTitle  READ todoTitle  NOTIFY dataChanged)
    Q_PROPERTY(QJsonArray todoTasks READ todoTasks WRITE setTodoTasks NOTIFY todoTasksChanged)
public:
    explicit PartObject(QObject *parent = nullptr);

    QString partId() const { return m_partId; }
    void setPartId(const QString &id);

    QString partType() const { return m_partType; }
    void setPartType(const QString &t);

    QString content() const { return m_content; }
    void setContent(const QString &c);

    QString callID() const { return m_callID; }
    void setCallID(const QString &id);

    QString toolType() const { return m_toolType; }
    void setToolType(const QString &t);

    QString toolName() const { return m_toolName; }
    void setToolName(const QString &n);

    int status() const { return m_status; }
    void setStatus(int s);

    bool isCommand() const { return m_isCommand; }
    void setIsCommand(bool v);

    QString shellType() const { return m_shellType; }
    void setShellType(const QString &s);

    QString output() const { return m_output; }
    void setOutput(const QString &o);

    int linesAdded() const { return m_linesAdded; }
    void setLinesAdded(int n);

    int linesRemoved() const { return m_linesRemoved; }
    void setLinesRemoved(int n);

    QString permissionRequestId() const { return m_permRequestId; }
    void setPermissionRequestId(const QString &id);

    QString permissionReply() const { return m_permReply; }

    QString todoListId() const { return m_todoListId; }
    void setTodoListId(const QString &id);

    QString todoTitle() const { return m_todoTitle; }
    void setTodoTitle(const QString &t);

    QJsonArray todoTasks() const { return m_todoTasks; }
    void setTodoTasks(const QJsonArray &tasks);

    /** Called from QML when user clicks Allow Once / Always / Reject. */
    Q_INVOKABLE void replyPermission(const QString &reply);

    /** Called from QML when user clicks on file name in edit/write cards. */
    Q_INVOKABLE void openFile();

signals:
    void dataChanged();
    void contentChanged();
    void outputChanged();
    void permissionReplyChanged();
    void todoTasksChanged();
    /** Emitted by replyPermission() — QmlMessageModel forwards to ChatService. */
    void permissionReplied(const QString &requestId, const QString &reply);

private:
    QString m_partId;        // server part id (opencode v1 part identity)
    QString m_partType;      // "thinking" | "text" | "tool" | "permission"
    QString m_content;
    QString m_callID;
    QString m_toolType;      // "cmd", "read", "write", "edit", "grep", "glob", "question"
    QString m_toolName;
    int     m_status = 0;    // 0=processing, 1=success, 2=failure
    bool    m_isCommand = false;
    QString m_shellType;
    QString m_output;
    int     m_linesAdded = 0;
    int     m_linesRemoved = 0;
    QString m_permRequestId;
    QString m_permReply;
    QString m_todoListId;
    QString m_todoTitle;
    QJsonArray m_todoTasks;
};

// ---------------------------------------------------------------------------
// Input struct for batch message operations
// ---------------------------------------------------------------------------
struct MsgInput {
    ChatMessageBubble::Type type;
    QString content;
    QString thinking;
    QString messageId;       // server message id (part-driven rows)
    QJsonArray parts;        // raw server parts (assistant only; part-driven rendering)
};

// ---------------------------------------------------------------------------
// QmlMessageModel
// ---------------------------------------------------------------------------
class QmlMessageModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
public:
    enum Roles {
        TypeRole = Qt::UserRole + 1,
        ContentRole,
        ThinkingRole,
        StreamingRole,
        PartsRole,
        PartsVersionRole,
        PermissionIdRole,
        PermissionReplyRole
    };

    explicit QmlMessageModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // ---- message management (backward-compatible API) ----
    void addMessage(ChatMessageBubble::Type type, const QString &content);
    void insertMessage(int row, ChatMessageBubble::Type type, const QString &content);
    void removeMessage(int row);
    void clearMessages();

    /** Replace all messages in one reset (for bulk load / prepend). */
    void resetMessages(const QList<MsgInput> &messages);

    /** Append multiple messages in one beginInsertRows/endInsertRows batch. */
    void appendMessages(const QList<MsgInput> &messages);

    /** Prepend older history messages at the top in one insert batch.
     *  Emits aboutToPrependMessages(count) BEFORE the rows are inserted so
     *  the QML view can suppress auto-scroll and re-anchor the viewport. */
    void prependMessages(const QList<MsgInput> &messages);

    int  messageCount() const { return m_messages.size(); }
    ChatMessageBubble::Type messageType(int row) const;
    QString messageContent(int row) const;
    QString messageThinking(int row) const;

    // ---- streaming API ----
    void beginStreaming();
    void appendStreamingText(const QString &delta);
    void appendStreamingThinking(const QString &content);
    void appendToolCall(const QString &callID, const QString &toolType,
                        const QString &toolName, const QString &input);
    void updateToolCallStatus(const QString &callID, int status, const QString &output);
    void endStreaming();

    // ---- part-driven API (opencode v1: SSE parts render by identity) ----

    /** Insert or update the part identified by partId inside the message
     *  identified by messageId (falls back to the last streaming assistant
     *  row, creating it when absent). Parts keep server arrival order.
     *  Non-displayable types (step-start/step-finish/...) are ignored. */
    void upsertPart(const QString &messageId, const QString &partId,
                    const QString &partType, const QJsonObject &part);

    /** Append a streaming delta to the text/thinking part identified by
     *  partId. Ignored when the part has not been announced yet. */
    void appendPartDelta(const QString &messageId, const QString &partId,
                         const QString &delta);

    // ---- todo plan (todo.updated SSE events drive the todo_write tool
    // part's TodoCard; the part itself comes from upsertPart/history) ----

    /** Update one task's status/output inside the todo_write tool card. */
    void todoUpdated(const QString &todoListId, const QString &taskId,
                     const QString &status, const QString &output);

    /** True when the row exists and carries at least one PartObject. */
    bool rowHasParts(int row) const;

    // ---- scroll helper (forwarded from QML) ----
    Q_INVOKABLE void scrollToBottom();

    /** Called from QML when user scrolls to top. */
    Q_INVOKABLE void requestLoadMore();

signals:
    void countChanged();
    void scrollToBottomRequested();
    void permissionReplied(const QString &requestId, const QString &reply);
    void loadMoreRequested();
    /** Emitted right before history rows are inserted at the top. */
    void aboutToPrependMessages(int count);

private:
    struct MessageEntry {
        ChatMessageBubble::Type type = ChatMessageBubble::Assistant;
        QString content;
        QString thinking;
        bool streaming = false;
        QString messageId;       // server message id (part-driven rows)
        bool partsManaged = false; // true: parts list is authoritative (server-driven); buildEntryParts must not rebuild it
        QList<PartObject*> parts;
        mutable QVariantList cachedPartsList; // cached QVariantList for QML (avoids re-creation on every access)
        int partsVersion = 0; // incremented on every rebuildParts, used to trigger QML updates without binding to parts

        struct ToolRef {
            QString callID;
            QString toolType;
            QString toolName;
            int status = 0;
            QString body;
            bool isCommand = false;
            QString shellType;
            QString output;
            int linesAdded = 0;
            int linesRemoved = 0;
        };
        QList<ToolRef> toolCalls;
    };

    void rebuildParts(int row);
    /** Build parts for an entry without touching the live model — used by
     *  bulk operations so delegates are created with final data in place. */
    void buildEntryParts(MessageEntry &msg);
    static void clearParts(QList<PartObject*> &parts);
    /** Connect PartObject signals to QmlMessageModel signals. */
    void connectPartSignals(PartObject *p);

    QList<MessageEntry> m_messages;
    QHash<QString, QPair<int,int>> m_callIdMap; // callID → (msgRow, toolIdx)
};

} // namespace ady

#endif // QML_MESSAGE_MODEL_H
