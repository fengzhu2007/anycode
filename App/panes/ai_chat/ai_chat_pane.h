#ifndef AI_CHAT_PANE_H
#define AI_CHAT_PANE_H

#include <docking_pane.h>
#include "core/event_bus/subscriber.h"
#include "chat_service.h"

namespace Ui {
class AIChatPane;
}

namespace ady{

class AIChatPanePrivate;

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

    static AIChatPane* open(DockingPaneManager* dockingManager, bool active = false);
    static AIChatPane* make(DockingPaneManager* dockingManager, const QJsonObject& data);
    static AIChatPane* getInstance();

public slots:
    void onActionTriggered();
    void onSessionSelected();
    void onSendMessage();
    void onSessionsReceived(const QList<OpenCodeSession> &sessions, const QString &error);
    void onSessionCreated(const OpenCodeSession &session, const QString &error);
    void onSessionDeleted(const QString &sessionId, const QString &error);
    void onStreamStarted(const QString &sessionId);
    void onStreamChunk(const QString &sessionId, const QString &delta);
    void onStreamFinished(const QString &sessionId, const QString &error);
    void onSessionStatusChanged(const QString &sessionId, const QString &status);
    void onModelsReceived(const QList<OpenCodeModel> &models, const QString &error);
    void onModelChanged(int index);

private:
    explicit AIChatPane(QWidget *parent = nullptr);

    void renderUserMessage(const QString &content);
    void appendEvent(const QString &text);
    void clearMessages();
    void scrollToBottom();

    void refreshSessionList();
    void loadSessionsFromServer();
    void setSending(bool sending);

    // 持久化模型选择偏好
    void saveModelPreference();
    void loadModelPreference();

    QString modelPreferencePath() const;

private:
    Ui::AIChatPane *ui;
    ChatService *m_service;
    AIChatPanePrivate *d;
    static AIChatPane* instance;

public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;
};

}

#endif // AI_CHAT_PANE_H
