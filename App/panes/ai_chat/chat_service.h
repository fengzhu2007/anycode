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
#include <curl/curl.h>

namespace ady{

/**
 * OpenCode Session 结构
 */
struct OpenCodeSession {
    QString id;           // ses_xxx
    QString title;
    QString agent;
    qint64 timeCreated;
    qint64 timeUpdated;
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
 * ChatService - OpenCode Server 原生 API 通信层
 *
 * 使用 opencode serve 的原生 API:
 * - /session - 会话管理
 * - /session/{id}/message - 发送消息
 * - /global/event - SSE 事件流
 * - /api/model - 模型列表
 * - /session/{id}/abort - 中止请求
 */
class ANYENGINE_EXPORT ChatService : public QObject
{
    Q_OBJECT
public:
    explicit ChatService(QObject *parent = nullptr);
    ~ChatService();

    // 服务器配置
    void setBaseUrl(const QString &url);
    QString baseUrl() const { return m_baseUrl; }

    // 当前选中的模型
    void setProviderID(const QString &id) { m_providerID = id; }
    void setModelID(const QString &id) { m_modelID = id; }
    QString providerID() const { return m_providerID; }
    QString modelID() const { return m_modelID; }

    // 当前会话
    void setCurrentSessionId(const QString &id) { m_currentSessionId = id; }
    QString currentSessionId() const { return m_currentSessionId; }

    // API 调用
    void listSessions();                    // GET /session
    void createSession(const QString &title = "");  // POST /session
    void deleteSession(const QString &sessionId);   // DELETE /session/{id}
    void listModels();                      // GET /api/model
    void sendMessage(const QString &sessionId, const QString &content);  // POST /session/{id}/message
    void abortSession(const QString &sessionId);  // POST /session/{id}/abort

    // 缓存的数据
    QList<OpenCodeSession> sessions() const { return m_sessions; }
    QList<OpenCodeModel> models() const { return m_models; }

signals:
    // 会话列表
    void sessionsReceived(const QList<OpenCodeSession> &sessions, const QString &error);
    void sessionCreated(const OpenCodeSession &session, const QString &error);
    void sessionDeleted(const QString &sessionId, const QString &error);

    // 模型列表
    void modelsReceived(const QList<OpenCodeModel> &models, const QString &error);

    // 消息发送
    void messageSent(const QString &sessionId, const QString &error);

    // SSE 事件流
    void streamStarted(const QString &sessionId);
    void streamChunk(const QString &sessionId, const QString &delta);
    void streamFinished(const QString &sessionId, const QString &error);
    void sessionStatusChanged(const QString &sessionId, const QString &status);

private:
    // SSE 事件流连接
    void connectEventStream();
    void disconnectEventStream();

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

    // 请求状态
    bool m_requesting;
    volatile bool m_abort;
    QFuture<void> m_future;

    // SSE 事件流
    CURL *m_eventCurl;
    QFuture<void> m_eventFuture;
    volatile bool m_eventAbort;
    QString m_sseBuffer;
};

}

#endif // CHAT_SERVICE_H
