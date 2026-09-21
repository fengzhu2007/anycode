#ifndef AI_CHAT_PANE_H
#define AI_CHAT_PANE_H

#include <docking_pane.h>
#include "core/event_bus/subscriber.h"
#include "chat_service.h"
#include "session_page_widget.h"
#include "chat_message_view.h"
#include <QTimer>
#include <QSet>
#include <QProcess>

namespace Ui {
class AIChatPane;
}

namespace ady{

class AIChatPanePrivate;
class SessionListPopup;

class AIChatPane : public DockingPane, public Subscriber
{
    Q_OBJECT

public:
    ~AIChatPane();

    void initView();
    virtual QString id() override;
    virtual QString group() override;
    virtual bool onReceive(Event* e) override;
    virtual QJsonObject toJson() override;

public:
    static AIChatPane* open(DockingPaneManager* dockingManager, bool active = false);
    static AIChatPane* make(DockingPaneManager* dockingManager, const QJsonObject& data);
    static AIChatPane* getInstance();

    /**
     * 停止 SSE 事件流和重连定时器，用于程序退出时清理
     */
    void shutdown();

    /**
     * 单个会话的运行时状态数据
     */
    struct SessionData {
        QString sessionId;
        SessionPageWidget* page = nullptr;
        bool isReceiving = false;
        QString pendingMessage;
        QString firstUserMessage;
        QString modelProviderID;
        QString modelID;
        QString directory;               // session working directory
        QString preference;              // session preference (system prompt text)
        QTimer *scrollTimer = nullptr; // throttled scroll-to-bottom during streaming
        int streamingRow = -1;         // model row of the streaming message (-1 = none)
        QString streamingContent;      // accumulated streaming text (survives virtualization)
        bool isStreaming = false;       // whether streaming is active (survives virtualization)
        bool permissionPending = false;  // whether a permission request is awaiting user reply
        int permissionRow = -1;          // model row of the permission widget
        // Pagination state for loading older messages
        qint64 oldestMessageTimestamp = 0;  // time_created of the oldest message currently loaded
        bool isLoadingMore = false;          // true while a "load older" request is in flight
        bool hasMoreMessages = true;         // false once a "load older" returns fewer than limit
    };

public slots:
    void onActionTriggered();
    void onSendMessage();
    void onSessionsReceived(const QList<OpenCodeSession> &sessions, const QString &error);
    void onSessionCreated(const OpenCodeSession &session, const QString &error);
    void onSessionDeleted(const QString &sessionId, const QString &error);
    void onStreamStarted(const QString &sessionId);
    void onStreamChunk(const QString &sessionId, const QString &delta);
    void onStreamThinking(const QString &sessionId, const QString &content);
    void onStreamToolUse(const QString &sessionId, const QString &callID, const QString &toolType, const QString &toolName, const QString &input);
    void onStreamToolResult(const QString &sessionId, const QString &callID, const QString &toolType, const QString &toolName, const QString &output);
    void onStreamFinished(const QString &sessionId, const QString &error);
    // part-driven rendering (opencode v1 part events)
    void onPartUpdated(const QString &sessionId, const QString &messageId,
                       const QString &partId, const QString &partType,
                       const QJsonObject &part);
    void onPartDelta(const QString &sessionId, const QString &messageId,
                     const QString &partId, const QString &delta);
    void onSessionStatusChanged(const QString &sessionId, const QString &status);
    void onSessionTitleChanged(const QString &sessionId, const QString &title);
    void onModelsReceived(const QList<OpenCodeModel> &models, const QString &error);
    void onMessagesReceived(const QString &sessionId, const QList<OpenCodeMessage> &messages, const QString &error);
    void onMessagesPrepended(const QString &sessionId, const QList<OpenCodeMessage> &olderMessages, const QString &error);
    void onCompactClicked();
    void onCompactionStarted(const QString &sessionId);
    void onCompactionFinished(const QString &sessionId);
    void onConnectionChanged(bool connected);
    void onPermissionAsked(const OpenCodePermissionRequest &request);
    void onMemorySaved(const QString &sessionId, const QString &type,
                       const QString &content, const QString &keywords);
    void onTodoUpdated(const QString &sessionId, const QString &todoListId,
                       const QString &taskId, const QString &status,
                       const QString &output);
    void onLoadMoreMessages(const QString &sessionId);

protected:
    virtual void closeEvent(QCloseEvent* e) override;

private:
    explicit AIChatPane(QWidget *parent = nullptr);

    // ---- session page management ----
    SessionPageWidget* createSessionPage();
    SessionPageWidget* findPage(const QString &sessionId) const;
    void removePage(SessionPageWidget *page);
    int findSessionIndex(const QString &sessionId) const;

    void switchToSession(const QString &sessionId);
    void refreshSessionPopup();
    void stopStreaming(const QString &sessionId);
    void throttledScrollToBottom(SessionData *sd);

    // ---- model combo (all pages) ----
    void updateAllModelCombos(const QList<OpenCodeModel> &models);

    // ---- message rendering (page-scoped) ----
    void renderUserMessage(SessionPageWidget *page, const QString &content);
    void appendEvent(SessionPageWidget *page, const QString &text);
    void clearMessages(SessionPageWidget *page);
    void scrollToBottom(SessionPageWidget *page);

    // ---- UI state (current page) ----
    void setSending(SessionPageWidget *page, bool sending);

    // ---- model preference persistence ----
    void saveModelPreference();
    void loadModelPreference();
    QString modelPreferencePath() const;

    // ---- workspace ----
    QString primaryWorkspacePath() const;
    QStringList allWorkspacePaths() const;
    QString resolveWorkspacePathForPath(const QString &path) const;
    void notifyWorkspacesChanged();

    // ---- opencode server auto-start (via terminal) ----
    void startServerIfNeeded();
    QString findServerExecutable();
    void onServerPingResult(bool ok);
    void appendEventToCurrentPage(const QString &text);

private:
    Ui::AIChatPane *ui;
    ChatService *m_service;
    AIChatPanePrivate *d;

    QList<SessionData*> m_sessions;
    QString m_currentSessionId;
    bool m_modelsLoaded = false;
    SessionListPopup *m_sessionPopup = nullptr;

    QTimer *m_workspaceNotifyTimer = nullptr;
    QTimer *m_retryLoadTimer = nullptr;     // retry listSessions on connection failure
    QStringList m_pendingDeleteIds;          // session IDs queued for deletion

    // opencode server auto-start state
    bool m_serverStartRequested = false;
    int m_pingCount = 0;
    uint16_t m_serverPort = 0;
    QProcess *m_serverProcess = nullptr;

    static AIChatPane* instance;

public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;

    /** Insert a debug test message containing thinking + tool calls + text. */
    void insertDebugTestMessage();
};

}

#endif // AI_CHAT_PANE_H
