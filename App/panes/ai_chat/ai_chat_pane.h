#ifndef AI_CHAT_PANE_H
#define AI_CHAT_PANE_H

#include <docking_pane.h>
#include "core/event_bus/subscriber.h"
#include "chat_service.h"
#include "session_page_widget.h"
#include <QTimer>

namespace Ui {
class AIChatPane;
}

namespace ady{

class AIChatPanePrivate;
class SessionListPopup;
class ChatMessageWidget;

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
     * 单个会话的运行时状态数据
     */
    struct SessionData {
        QString sessionId;
        SessionPageWidget* page = nullptr;
        bool isReceiving = false;
        ChatMessageWidget* streamingWidget = nullptr;
        QString pendingMessage;
        QString firstUserMessage;
        QString modelProviderID;
        QString modelID;
    };

public slots:
    void onActionTriggered();
    void onSendMessage();
    void onSessionsReceived(const QList<OpenCodeSession> &sessions, const QString &error);
    void onSessionCreated(const OpenCodeSession &session, const QString &error);
    void onSessionDeleted(const QString &sessionId, const QString &error);
    void onStreamStarted(const QString &sessionId);
    void onStreamChunk(const QString &sessionId, const QString &delta);
    void onStreamFinished(const QString &sessionId, const QString &error);
    void onSessionStatusChanged(const QString &sessionId, const QString &status);
    void onModelsReceived(const QList<OpenCodeModel> &models, const QString &error);
    void onMessagesReceived(const QString &sessionId, const QList<OpenCodeMessage> &messages, const QString &error);
    void onCompactClicked();
    void onCompactionStarted(const QString &sessionId);
    void onCompactionFinished(const QString &sessionId);

private:
    explicit AIChatPane(QWidget *parent = nullptr);

    // ---- session page management ----
    SessionPageWidget* createSessionPage();
    SessionPageWidget* findPage(const QString &sessionId) const;
    void removePage(SessionPageWidget *page);
    int findSessionIndex(const QString &sessionId) const;

    void switchToSession(const QString &sessionId);
    void refreshSessionPopup();

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
    void notifyWorkspacesChanged();

private:
    Ui::AIChatPane *ui;
    ChatService *m_service;
    AIChatPanePrivate *d;

    QList<SessionData*> m_sessions;
    QString m_currentSessionId;
    bool m_modelsLoaded = false;
    SessionListPopup *m_sessionPopup = nullptr;

    QTimer *m_workspaceNotifyTimer = nullptr;
    static AIChatPane* instance;

public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;
};

}

#endif // AI_CHAT_PANE_H
