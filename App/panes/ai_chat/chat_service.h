#ifndef CHAT_SERVICE_H
#define CHAT_SERVICE_H

#include "global.h"
#include <QObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QFuture>
#include <QMap>
#include <QHash>
#include <QTimer>
#include <curl/curl.h>

namespace ady{

/**
 * OpenCode Session 结构
 */
struct OpenCodeSession {
    QString id;           // ses_xxx
    QString title;
    QString agent;
    QString directory;
    QString preference;
    qint64 timeCreated;
    qint64 timeUpdated;
    QString modelProviderID;
    QString modelID;
};
Q_DECLARE_METATYPE(OpenCodeSession)
Q_DECLARE_METATYPE(QList<OpenCodeSession>)

/**
 * OpenCode Model 结构
 */
struct OpenCodeModel {
    QString providerID;   // e.g. "openai"
    QString modelID;      // e.g. "gpt-4"
    QString name;         // display name
};
Q_DECLARE_METATYPE(OpenCodeModel)
Q_DECLARE_METATYPE(QList<OpenCodeModel>)

/**
 * OpenCode 历史消息结构
 */
struct OpenCodeMessage {
    QString id;           // msg_xxx
    QString role;         // "user" | "assistant"
    QString text;
    QString thinking;     // reasoning content (separate from text)
    QJsonArray parts;     // raw parts array (part-driven rendering); text/thinking kept flattened for compatibility
    qint64 timeCreated;
};
Q_DECLARE_METATYPE(OpenCodeMessage)
Q_DECLARE_METATYPE(QList<OpenCodeMessage>)

/**
 * OpenCode 权限请求结构（permission.asked 事件）
 */
struct OpenCodePermissionRequest {
    QString id;              // 权限请求 ID
    QString sessionId;       // 所属会话
    QString permission;      // 权限类型（如 "glob", "bash"）
    QString toolName;        // 触发工具名
    QStringList patterns;    // 匹配模式
    QJsonObject metadata;    // 额外上下文（command、filePath 等）
};
Q_DECLARE_METATYPE(OpenCodePermissionRequest)

/**
 * 文件变更信息（session.diff 事件）
 */
struct FileDiffInfo {
    QString file;            // 文件路径
    QString status;          // "added" | "deleted" | "modified"
    int additions = 0;       // 新增行数
    int deletions = 0;       // 删除行数
};
Q_DECLARE_METATYPE(FileDiffInfo)
Q_DECLARE_METATYPE(QList<FileDiffInfo>)

/**
 * ChatService - OpenCode Server 原生 API 通信层
 *
 * 使用 opencode serve 的原生 API:
 * - /session - 会话管理
 * - /session/{id}/message - 发送消息
 * - /global/event - SSE 事件流
 * - /api/model - 模型列表
 * - /session/{id}/abort - 中止请求
 * - /permission/{id}/reply - 回复权限请求
 */
class ANYENGINE_EXPORT ChatService : public QObject
{
    Q_OBJECT
public:
    explicit ChatService(QObject *parent = nullptr);
    ~ChatService();

    /** 获取 opencode-cpp 服务端口号（可通过 setServerPort 在运行时修改） */
    static int serverPort() { return s_serverPort; }
    void setServerPort(int port);

    void setBaseUrl(const QString &url);
    QString baseUrl() const { return m_baseUrl; }

    /**
     * 异步健康检查：GET /session（短超时）
     * 结果通过 pingResult 信号返回（在主线程接收）
     */
    void pingServer();

    void setProviderID(const QString &id) { m_providerID = id; }
    void setModelID(const QString &id) { m_modelID = id; }
    QString providerID() const { return m_providerID; }
    QString modelID() const { return m_modelID; }

    void setCurrentSessionId(const QString &id) { m_currentSessionId = id; }
    QString currentSessionId() const { return m_currentSessionId; }

    void listSessions();                    // GET /session
    void createSession(const QString &title = "", const QString &directory = "", const QString &providerID = QString(), const QString &modelID = "");  // POST /session
    void deleteSession(const QString &sessionId);   // DELETE /session/{id}
    void listModels();                      // GET /api/model
    bool sendMessage(const QString &sessionId, const QString &content, const QString &providerID = QString(), const QString &modelID = QString());  // POST /session/{id}/message (false = request dropped by in-flight lock)
    void loadSessionMessages(const QString &sessionId, int limit = 20, qint64 beforeTimestamp = 0);  // GET /session/{id}/message
    void abortSession(const QString &sessionId);  // POST /session/{id}/abort
    void updateSession(const QString &sessionId, const QString &title,
                       const QString &directory, const QString &preference);  // PATCH /session/{id}
    void setWorkingDirectories(const QStringList &directories);  // POST /directories (global)
    void compactSession(const QString &sessionId);  // POST /api/session/{id}/compact
    void confirmChanges(const QString &sessionId);   // POST /session/{id}/changes/confirm
    void revertSession(const QString &sessionId);    // POST /session/{id}/revert
    void revertCommitSession(const QString &sessionId);  // POST /session/{id}/revert/commit

    /**
     * 回复权限请求：POST /permission/{id}/reply
     * @param requestId 权限请求 ID
     * @param reply     "once" | "always" | "reject"
     */
    void replyPermission(const QString &requestId, const QString &reply);

    void setGatewayPort(uint16_t port) { m_gatewayPort = port; }
    uint16_t gatewayPort() const { return m_gatewayPort; }

    /**
     * 停止 SSE 事件流和所有定时器，用于程序退出时清理
     */
    void shutdown();

    QList<OpenCodeSession> sessions() const { return m_sessions; }
    QList<OpenCodeModel> models() const { return m_models; }

signals:
    void sessionsReceived(const QList<OpenCodeSession> &sessions, const QString &error);
    void sessionCreated(const OpenCodeSession &session, const QString &error);
    void sessionDeleted(const QString &sessionId, const QString &error);

    void modelsReceived(const QList<OpenCodeModel> &models, const QString &error);

    void messageSent(const QString &sessionId, const QString &error);

    void messagesReceived(const QString &sessionId, const QList<OpenCodeMessage> &messages, const QString &error);
    void messagesPrepended(const QString &sessionId, const QList<OpenCodeMessage> &olderMessages, const QString &error);

    void streamStarted(const QString &sessionId);
    void streamChunk(const QString &sessionId, const QString &delta);
    void streamThinking(const QString &sessionId, const QString &content);
    void streamToolUse(const QString &sessionId, const QString &callID, const QString &toolType, const QString &toolName, const QString &input);
    void streamToolResult(const QString &sessionId, const QString &callID, const QString &toolType, const QString &toolName, const QString &output);
    void streamFinished(const QString &sessionId, const QString &error);

    /** Part-level SSE events (opencode v1 part-driven rendering).
     *  partUpdated fires for every message.part.updated announcement with the
     *  raw part JSON; partDelta fires for every message.part.delta chunk.
     *  Only assistant-message parts are emitted (user parts would duplicate
     *  the locally rendered user bubble). */
    void partUpdated(const QString &sessionId, const QString &messageId,
                     const QString &partId, const QString &partType,
                     const QJsonObject &part);
    void partDelta(const QString &sessionId, const QString &messageId,
                   const QString &partId, const QString &delta);

    void sessionStatusChanged(const QString &sessionId, const QString &status);
    void sessionTitleChanged(const QString &sessionId, const QString &title);
    void sessionDiffChanged(const QString &sessionId, const QString &summary);
    void sessionDiffReceived(const QString &sessionId, const QList<FileDiffInfo> &diffs);

    void compactionStarted(const QString &sessionId);
    void compactionFinished(const QString &sessionId);
    void autoCompactionTriggered(const QString &sessionId);

    void permissionAsked(const OpenCodePermissionRequest &request);

    void memorySaved(const QString &sessionId, const QString &type,
                     const QString &content, const QString &keywords);

    void todoUpdated(const QString &sessionId, const QString &todoListId,
                     const QString &taskId, const QString &status,
                     const QString &output);

    void connectionChanged(bool connected);
    void pingResult(bool ok);
    void eventStreamEnded(bool wasConnected);  // internal: event stream thread ended

private:
    void connectEventStream();
    void disconnectEventStream();

    void registerMcpServer();

private slots:
    void onEventStreamEnded(bool wasConnected);
    void scheduleReconnect();

    // curl callbacks
    static size_t writeCallback(void *ptr, size_t size, size_t nmemb, void *userdata);
    static size_t eventStreamCallback(void *ptr, size_t size, size_t nmemb, void *userdata);
    static int progressCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow,
                                curl_off_t ultotal, curl_off_t ulnow);

    CURL* createCurlHandle();
    void processEventStream(const QByteArray &chunk);

private:
    QString m_baseUrl;
    QString m_providerID;
    QString m_modelID;
    QString m_currentSessionId;

    QList<OpenCodeSession> m_sessions;
    QList<OpenCodeModel> m_models;

    bool m_requesting;
    volatile bool m_abort;
    QFuture<void> m_future;

    CURL *m_eventCurl;
    QFuture<void> m_eventFuture;
    volatile bool m_eventAbort;
    QString m_sseBuffer;

    QMap<QString, QString> m_sessionErrors;  // sessionId → error message

    uint16_t m_gatewayPort = 3456;
    bool m_mcpRegistered = false;

    QMap<QString, qint64> m_sessionContentSizes;  // sessionId → accumulated bytes
    static const qint64 COMPACT_THRESHOLD = 200 * 1024;  // 200KB

    // partID → part type, learned from message.part.updated; used to route
    // message.part.delta events (reasoning deltas render as thinking)
    QHash<QString, QString> m_partTypes;
    // partID → messageID, learned from message.part.updated; lets partDelta
    // reach the model row that owns the part
    QHash<QString, QString> m_partMessages;
    // messageID → role, learned from message.updated; part-level events are
    // suppressed for user messages (already rendered locally)
    QHash<QString, QString> m_messageRoles;
    // childSessionId → parentSessionId, learned from subsession.started;
    // routes child session events to the parent page for rendering
    QHash<QString, QString> m_childToParent;
    // partID → accumulated reasoning text (appendThink replaces, so each
    // reasoning delta re-emits the accumulated full text)
    QHash<QString, QString> m_reasoningTexts;

    bool m_connected = false;
    QTimer *m_reconnectTimer = nullptr;
    int m_reconnectRetry = 0;

    static int s_serverPort;
};

}

#endif // CHAT_SERVICE_H
