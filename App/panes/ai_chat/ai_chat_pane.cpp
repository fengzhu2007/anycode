#include "ai_chat_pane.h"
#include "ui_ai_chat_pane.h"
#include "components/message_dialog.h"
#include "qml_message_model.h"
#include "session_list_popup.h"
#include "docking_pane_layout_item_info.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event.h"
#include "core/event_bus/event_data.h"
#include "core/event_bus/publisher.h"
#include "core/theme.h"
#include "panes/resource_manager/resource_manager_model.h"
#include "panes/resource_manager/resource_manager_model_item.h"
#include "modules/options/options_settings.h"
#include "modules/options/network_settings.h"
#include <QElapsedTimer>
#include "modules/options/agent_settings.h"

#include <QDir>

#include <QAction>
#include <QScrollBar>
#include <QMetaObject>
#include <QKeyEvent>
#include <QPushButton>
#include <QToolButton>
#include <QToolBar>
#include <QTextEdit>
#include <QComboBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDebug>
#include <QQuickWindow>
#include <algorithm>

namespace ady{

AIChatPane* AIChatPane::instance = nullptr;

const QString AIChatPane::PANE_ID = "AIChat";
const QString AIChatPane::PANE_GROUP = "AIChat";

class AIChatPanePrivate{
public:
};

// ---- construction / destruction ----

AIChatPane::AIChatPane(QWidget *parent)
    : DockingPane(parent)
    , Subscriber()
    , ui(new Ui::AIChatPane)
{
    QWidget* widget = new QWidget(this);
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(tr("AI Chat"));


    QQuickWindow::setTextRenderType(QQuickWindow::NativeTextRendering);

    // theme styling
    QString borderColor = Theme::getInstance()->borderColor().name(QColor::HexRgb);
    this->setStyleSheet(
        "QToolBar{border:0px;}"
        "QTextEdit{border:1px solid " + borderColor + ";}"
        "QComboBox{border:1px solid " + borderColor + ";padding:2px;}"
    );

    d = new AIChatPanePrivate;

    // create chat service
    m_service = new ChatService(this);


    // sessions popup
    m_sessionPopup = new SessionListPopup(this);
    connect(m_sessionPopup, &SessionListPopup::sessionClicked, [this](const QString &sessionId){
        switchToSession(sessionId);
    });
    connect(m_sessionPopup, &SessionListPopup::sessionCloseClicked, [this](const QString &sessionId){
        m_service->deleteSession(sessionId);
    });

    // toolbar connections
    connect(ui->actionNewChat, &QAction::triggered, this, &AIChatPane::onActionTriggered);
    connect(ui->actionDeleteChat, &QAction::triggered, this, &AIChatPane::onActionTriggered);
    connect(ui->actionDelete_All_Chat, &QAction::triggered, this, &AIChatPane::onActionTriggered);
    connect(ui->actionClear, &QAction::triggered, this, &AIChatPane::onActionTriggered);
    connect(ui->actionRefreshModels, &QAction::triggered, this, &AIChatPane::onActionTriggered);
    connect(ui->actionSessions, &QAction::triggered, [this](){
        refreshSessionPopup();
        QPoint topLeft = ui->stackedWidget->mapToGlobal(QPoint(0, 0));
        int width = ui->stackedWidget->width();
        m_sessionPopup->setFixedWidth(width);
        m_sessionPopup->showAt(topLeft);
    });
    // service signals
    connect(m_service, &ChatService::sessionsReceived, this, &AIChatPane::onSessionsReceived);
    connect(m_service, &ChatService::sessionCreated, this, &AIChatPane::onSessionCreated);
    connect(m_service, &ChatService::sessionDeleted, this, &AIChatPane::onSessionDeleted);
    connect(m_service, &ChatService::streamStarted, this, &AIChatPane::onStreamStarted);
    connect(m_service, &ChatService::streamChunk, this, &AIChatPane::onStreamChunk);
    connect(m_service, &ChatService::streamThinking, this, &AIChatPane::onStreamThinking);
    connect(m_service, &ChatService::streamToolUse, this, &AIChatPane::onStreamToolUse);
    connect(m_service, &ChatService::streamToolResult, this, &AIChatPane::onStreamToolResult);
    connect(m_service, &ChatService::streamFinished, this, &AIChatPane::onStreamFinished);
    connect(m_service, &ChatService::sessionStatusChanged, this, &AIChatPane::onSessionStatusChanged);
    connect(m_service, &ChatService::sessionTitleChanged, this, &AIChatPane::onSessionTitleChanged);
    connect(m_service, &ChatService::sessionDiffChanged, this, [this](const QString &sessionId, const QString &summary){
        auto *page = findPage(sessionId);
        if(page){
            appendEvent(page, summary);
        }
    });
    connect(m_service, &ChatService::sessionDiffReceived, this, [this](const QString &sessionId, const QList<FileDiffInfo> &diffs){
        auto *page = findPage(sessionId);
        if(page){
            page->setFileDiffs(diffs);
        }
    });
    connect(m_service, &ChatService::modelsReceived, this, &AIChatPane::onModelsReceived);
    connect(m_service, &ChatService::messagesReceived, this, &AIChatPane::onMessagesReceived);
    connect(m_service, &ChatService::messagesPrepended, this, &AIChatPane::onMessagesPrepended);
    connect(m_service, &ChatService::compactionStarted, this, &AIChatPane::onCompactionStarted);
    connect(m_service, &ChatService::compactionFinished, this, &AIChatPane::onCompactionFinished);
    connect(m_service, &ChatService::autoCompactionTriggered, this, [this](const QString &sessionId){
        auto *page = findPage(sessionId);
        if(page){
            appendEvent(page, tr("Session exceeds 200KB, auto-compacting..."));
        }
    });
    connect(m_service, &ChatService::connectionChanged, this, &AIChatPane::onConnectionChanged);
    connect(m_service, &ChatService::pingResult, this, &AIChatPane::onServerPingResult);
    connect(m_service, &ChatService::permissionAsked, this, &AIChatPane::onPermissionAsked);
    connect(m_service, &ChatService::memorySaved, this, &AIChatPane::onMemorySaved);

    Subscriber::reg();
    this->regMessageIds({Type::M_OPEN_PROJECT, Type::M_CLOSE_PROJECT,
                         Type::M_ADD_TO_CHAT, Type::M_ADD_TO_NEW_CHAT});

    // workspace change debounce timer: 2s delay after last project open/close
    m_workspaceNotifyTimer = new QTimer(this);
    m_workspaceNotifyTimer->setSingleShot(true);
    m_workspaceNotifyTimer->setInterval(2000);
    connect(m_workspaceNotifyTimer, &QTimer::timeout, this, &AIChatPane::notifyWorkspacesChanged);

    // session list retry timer: retry listSessions when server is unreachable
    m_retryLoadTimer = new QTimer(this);
    m_retryLoadTimer->setSingleShot(true);
    m_retryLoadTimer->setInterval(5000);
    connect(m_retryLoadTimer, &QTimer::timeout, this, [this](){
        m_service->listSessions();
    });

    this->initView();
}

AIChatPane::~AIChatPane(){
    Subscriber::unReg();
    saveModelPreference();
    instance = nullptr;

    qDeleteAll(m_sessions);
    m_sessions.clear();

    delete m_service;
    m_service = nullptr;

    delete d;
    delete ui;
}

void AIChatPane::initView(){
    // load model preference
    loadModelPreference();

    // load sessions from server first, then listModels will be called in onSessionsReceived
    m_service->listSessions();
}

// ---- DockingPane overrides ----

QString AIChatPane::id(){
    return AIChatPane::PANE_ID;
}

QString AIChatPane::group(){
    return AIChatPane::PANE_GROUP;
}

bool AIChatPane::onReceive(Event* e){
    const QString id = e->id();
    if(id == Type::M_OPEN_PROJECT || id == Type::M_CLOSE_PROJECT){
        m_workspaceNotifyTimer->start();
        return true;
    }
    if(id == Type::M_ADD_TO_CHAT || id == Type::M_ADD_TO_NEW_CHAT){
        QString text;
        QJsonObject obj = e->toJsonOf<AiChatData>().toObject();
        text = obj["text"].toString();
        if(text.isEmpty()) return true;
        if(id == Type::M_ADD_TO_NEW_CHAT){
            // Create session page immediately so text can be filled in
            auto *page = createSessionPage();
            auto *sd = new SessionData;
            sd->page = page;
            m_sessions.append(sd);
            // Read model from page's combobox
            QString pid, mid;
            QString data = page->modelCombo()->currentData().toString();
            QStringList parts = data.split("::");
            if(parts.size() == 2){
                pid = parts[0];
                mid = parts[1];
            }
            m_service->createSession("", primaryWorkspacePath(), pid, mid);
            page->appendInputText(text + " ");
        }else{
            auto *page = findPage(m_currentSessionId);
            if(page){
                page->appendInputText(text + " ");
            }
        }
        return true;
    }
    return false;
}

QJsonObject AIChatPane::toJson(){
    return {
        {"id", this->id()},
        {"group", this->group()},
        {"data", QJsonObject{}}
    };
}

// ---- session management ----

void AIChatPane::onActionTriggered(){
    auto sender = static_cast<QAction*>(this->sender());
    if(sender == ui->actionNewChat){
        // Read model from current page's combobox
        QString pid, mid;
        auto *page = findPage(m_currentSessionId);
        if(page){
            QString data = page->modelCombo()->currentData().toString();
            QStringList parts = data.split("::");
            if(parts.size() == 2){
                pid = parts[0];
                mid = parts[1];
            }
        }
        m_service->createSession("", primaryWorkspacePath(), pid, mid);
    } else if(sender == ui->actionDeleteChat){
        if(!m_currentSessionId.isEmpty()){
            if(MessageDialog::confirm(this, tr("Delete Confirm"),
                    tr("Are you sure you want to delete the current chat?"),
                    QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok){
                m_service->deleteSession(m_currentSessionId);
            }
        }
    } else if(sender == ui->actionDelete_All_Chat){
        auto allSessions = m_service->sessions();
        if(!allSessions.isEmpty()){
            if(MessageDialog::confirm(this, tr("Delete All Confirm"),
                    tr("Are you sure you want to delete all %1 chat(s)?").arg(allSessions.size()),
                    QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok){
                m_pendingDeleteIds.clear();
                for(const auto &s : allSessions){
                    m_pendingDeleteIds << s.id;
                }
                if(!m_pendingDeleteIds.isEmpty()){
                    m_service->deleteSession(m_pendingDeleteIds.takeFirst());
                }
            }
        }
    } else if(sender == ui->actionClear){
        auto *page = findPage(m_currentSessionId);
        if(page) clearMessages(page);
    } else if(sender == ui->actionRefreshModels){
        m_service->listModels();
        // DEBUG: insert test bubble to verify tool rendering
        // TODO: remove after debugging
        insertDebugTestMessage();
    }
}

void AIChatPane::onSessionsReceived(const QList<OpenCodeSession> &sessions, const QString &error){
    if(!error.isEmpty()){
        qDebug() << "[AIChatPane] Load sessions failed:" << error
                 << "- retrying in 5s";
        // server not reachable - auto-start opencode-cpp via terminal (once)
        startServerIfNeeded();
        m_retryLoadTimer->start();
        return;
    }

    // Stop retry timer on success
    m_retryLoadTimer->stop();

    for(int i = m_sessions.size() - 1; i >= 0; --i){
        bool found = false;
        for(const auto &s : sessions){
            if(m_sessions.at(i)->sessionId == s.id){
                found = true;
                m_sessions.at(i)->modelProviderID = s.modelProviderID;
                m_sessions.at(i)->modelID = s.modelID;
                break;
            }
        }
        if(!found){
            auto *sd = m_sessions.at(i);
            removePage(sd->page);
            m_sessions.removeAt(i);
            delete sd;
        }
    }

    refreshSessionPopup();

    // Set working directories (global, once)
    QStringList paths = allWorkspacePaths();
    if (!paths.isEmpty()) {
        m_service->setWorkingDirectories(paths);
    }

    if(!sessions.isEmpty() && m_currentSessionId.isEmpty()){
        switchToSession(sessions.first().id);
    }

    if(!m_modelsLoaded){
        m_service->listModels();
    }
}

void AIChatPane::onSessionCreated(const OpenCodeSession &session, const QString &error){
    if(!error.isEmpty()){
        qDebug() << "[AIChatPane] Create session failed:" << error;
        for(auto *sd : m_sessions){
            sd->pendingMessage.clear();
        }
        return;
    }

    // Check if page was already created (e.g. from M_ADD_TO_NEW_CHAT)
    SessionData *sd = nullptr;
    SessionPageWidget *page = nullptr;
    for(auto *s : m_sessions){
        if(s->sessionId.isEmpty()){
            sd = s;
            page = s->page;
            sd->sessionId = session.id;
            page->setSessionId(session.id);
            // Read model from combobox if not set
            if(sd->modelProviderID.isEmpty() || sd->modelID.isEmpty()){
                QString data = page->modelCombo()->currentData().toString();
                QStringList parts = data.split("::");
                if(parts.size() == 2){
                    sd->modelProviderID = parts[0];
                    sd->modelID = parts[1];
                }
            }
            break;
        }
    }

    // Otherwise create new page
    if(!sd){
        page = createSessionPage();
        sd = new SessionData;
        sd->sessionId = session.id;
        sd->page = page;
        page->setSessionId(session.id);
        sd->modelProviderID = session.modelProviderID;
        sd->modelID = session.modelID;
        m_sessions.append(sd);
    }

    switchToSession(session.id);
    refreshSessionPopup();

    // Set working directories (global, once)
    QStringList paths = allWorkspacePaths();
    if (!paths.isEmpty()) {
        m_service->setWorkingDirectories(paths);
    }

    if(!sd->pendingMessage.isEmpty()){
        QString text = sd->pendingMessage;
        sd->pendingMessage.clear();

        renderUserMessage(page, text);
        setSending(page, true);

        if(!m_service->sendMessage(session.id, text, sd->modelProviderID, sd->modelID)){
            // request dropped because another request is still in flight
            setSending(page, false);
            appendEvent(page, tr("Previous request is still in progress, please retry"));
        }
    }
}

void AIChatPane::onSessionDeleted(const QString &sessionId, const QString &error){
    if(!error.isEmpty()){
        qDebug() << "[AIChatPane] Delete session failed:" << error;
    } else {
        int idx = findSessionIndex(sessionId);
        if(idx >= 0){
            auto *sd = m_sessions.at(idx);
            bool wasCurrent = (m_currentSessionId == sessionId);

            if(wasCurrent){
                m_currentSessionId.clear();
            }

            removePage(sd->page);
            if(sd->scrollTimer){
                sd->scrollTimer->stop();
                sd->scrollTimer->deleteLater();
            }
            m_sessions.removeAt(idx);
            delete sd;

            refreshSessionPopup();

            if(wasCurrent && !m_sessions.isEmpty()){
                // Switch to previous session, or next if no previous
                int targetIdx = (idx > 0) ? idx - 1 : 0;
                switchToSession(m_sessions.at(targetIdx)->sessionId);
            }
        }
    }

    // Continue deleting pending sessions (delete-all operation)
    if(!m_pendingDeleteIds.isEmpty()){
        m_service->deleteSession(m_pendingDeleteIds.takeFirst());
    }
}

// ---- page management ----

SessionPageWidget* AIChatPane::createSessionPage()
{
    auto *page = new SessionPageWidget(ui->stackedWidget);
    page->setChatService(m_service);

    QString borderColor = Theme::getInstance()->borderColor().name(QColor::HexRgb);
    page->messageInput()->setStyleSheet("QTextEdit{border:1px solid " + borderColor + ";}");
    page->modelCombo()->setStyleSheet("QComboBox{border:1px solid " + borderColor + ";padding:2px;}");

    connect(page, &SessionPageWidget::enterPressed, [this, page](){
        if(page == findPage(m_currentSessionId)){
            onSendMessage();
        }
    });

    connect(page->sendBtn(), &QToolButton::clicked, [this, page](){
        if(page != findPage(m_currentSessionId)) return;
        int idx = findSessionIndex(m_currentSessionId);
        if(idx < 0) return;
        auto *sd = m_sessions.at(idx);
        if(sd->isReceiving){
            if(!m_currentSessionId.isEmpty()){
                m_service->abortSession(m_currentSessionId);
            }
            stopStreaming(m_currentSessionId);
        }else{
            onSendMessage();
        }
    });

    connect(page->modelCombo(), QOverload<int>::of(&QComboBox::currentIndexChanged), [this, page](int index){
        if(index < 0) return;
        if(page != findPage(m_currentSessionId)) return;
        QString data = page->modelCombo()->itemData(index).toString();
        QStringList parts = data.split("::");
        if(parts.size() == 2){
            m_service->setProviderID(parts[0]);
            m_service->setModelID(parts[1]);
            saveModelPreference();
            int sdIdx = findSessionIndex(m_currentSessionId);
            if(sdIdx >= 0){
                m_sessions.at(sdIdx)->modelProviderID = parts[0];
                m_sessions.at(sdIdx)->modelID = parts[1];
            }
            appendEvent(page, tr("Switched to: %1").arg(page->modelCombo()->currentText()));
        }
    });

    auto models = m_service->models();
    if(!models.isEmpty()){
        QString defaultData = m_service->providerID() + "::" + m_service->modelID();
        page->setModels(models, defaultData);
    }

    // QML model: permission replies are handled via QmlMessageModel::permissionReplied
    // signal, which is connected in SessionPageWidget::setupQmlView().
    // No widgetCreated signal needed — QML handles its own rendering.

    ui->stackedWidget->addWidget(page);
    return page;
}

SessionPageWidget* AIChatPane::findPage(const QString &sessionId) const
{
    for(auto *sd : m_sessions){
        if(sd->sessionId == sessionId) return sd->page;
    }
    return nullptr;
}

void AIChatPane::removePage(SessionPageWidget *page)
{
    if(!page) return;
    ui->stackedWidget->removeWidget(page);
    delete page;
}

int AIChatPane::findSessionIndex(const QString &sessionId) const
{
    for(int i = 0; i < m_sessions.size(); ++i){
        if(m_sessions.at(i)->sessionId == sessionId) return i;
    }
    return -1;
}

void AIChatPane::switchToSession(const QString &sessionId)
{
    auto *page = findPage(sessionId);
    if(!page){
        page = createSessionPage();
        auto *sd = new SessionData;
        sd->sessionId = sessionId;
        sd->page = page;
        page->setSessionId(sessionId);
        // Copy model info from service's session list
        for(const auto &s : m_service->sessions()){
            if(s.id == sessionId){
                sd->modelProviderID = s.modelProviderID;
                sd->modelID = s.modelID;
                break;
            }
        }
        m_sessions.append(sd);

        // Connect scroll-to-top signal for loading older messages
        connect(page, &SessionPageWidget::scrollToTopRequested, this, [this, sessionId]() {
            onLoadMoreMessages(sessionId);
        });

        // Connect QML model's loadMoreRequested signal (from scroll-to-top in QML)
        connect(page->qmlModel(), &QmlMessageModel::loadMoreRequested, this, [this, sessionId]() {
            qDebug() << "[AIChatPane] loadMoreRequested from QML model for session" << sessionId;
            onLoadMoreMessages(sessionId);
        });

        m_service->loadSessionMessages(sessionId);
    }

    m_currentSessionId = sessionId;
    m_service->setCurrentSessionId(sessionId);

    int idx = findSessionIndex(sessionId);
    if(idx >= 0){
        auto *sd = m_sessions.at(idx);
        sd->firstUserMessage.clear();
        if(!sd->modelProviderID.isEmpty() && !sd->modelID.isEmpty()){
            QString modelData = sd->modelProviderID + "::" + sd->modelID;
            auto *combo = page->modelCombo();
            for(int i = 0; i < combo->count(); ++i){
                if(combo->itemData(i).toString() == modelData){
                    combo->setCurrentIndex(i);
                    break;
                }
            }
        }
    }

    ui->stackedWidget->setCurrentWidget(page);

    // Update session title label
    for(const auto &s : m_service->sessions()){
        if(s.id == sessionId){
            page->sessionTitle()->setText(s.title.isEmpty() ? tr("New Chat") : s.title);
            break;
        }
    }

    page->messageInput()->setFocus();
}

void AIChatPane::refreshSessionPopup()
{
    m_sessionPopup->refresh(m_service->sessions(), m_currentSessionId);
}

// ---- history messages ----

void AIChatPane::onMessagesReceived(const QString &sessionId, const QList<OpenCodeMessage> &messages, const QString &error){
    QElapsedTimer totalTimer;
    totalTimer.start();
    qDebug() << "[AIChatPane] onMessagesReceived: session=" << sessionId
             << "messages=" << messages.size() << "error=" << error;
    if(!error.isEmpty()){
        qDebug() << "[AIChatPane] Load messages error:" << error;
        return;
    }

    auto *page = findPage(sessionId);
    if(!page){
        qDebug() << "[AIChatPane] onMessagesReceived: page NOT found for session" << sessionId;
        return;
    }

    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0){
        qDebug() << "[AIChatPane] onMessagesReceived: session index NOT found";
        return;
    }
    auto *sd = m_sessions.at(sdIdx);

    // Build message list and reset model in one batch
    QElapsedTimer t;
    t.start();
    QList<MsgInput> allMessages;
    allMessages.reserve(messages.size());
    for(const auto &msg : messages){
        MsgInput mi;
        mi.type = (msg.role == "user")
            ? ChatMessageView::User
            : ChatMessageView::Assistant;
        mi.content = msg.text;
        mi.thinking = msg.thinking;
        allMessages.append(mi);
    }
    qDebug() << "[AIChatPane] build MsgInput:" << t.elapsed() << "ms for" << allMessages.size() << "messages";

    t.restart();
    qDebug() << "[AIChatPane] calling resetMessages...";
    page->qmlModel()->resetMessages(allMessages);
    qDebug() << "[AIChatPane] resetMessages done:" << t.elapsed() << "ms, rowCount=" << page->qmlModel()->rowCount();

    // Track oldest message timestamp for pagination
    if(!messages.isEmpty()){
        sd->oldestMessageTimestamp = messages.first().timeCreated;
        sd->hasMoreMessages = true;
    } else {
        sd->hasMoreMessages = false;
    }
    sd->isLoadingMore = false;

    t.restart();
    page->scrollToBottom();
    qDebug() << "[AIChatPane] scrollToBottom:" << t.elapsed() << "ms";

    qDebug() << "[AIChatPane] onMessagesReceived TOTAL:" << totalTimer.elapsed() << "ms";
}

void AIChatPane::onMessagesPrepended(const QString &sessionId, const QList<OpenCodeMessage> &olderMessages, const QString &error){
    qDebug() << "[AIChatPane] onMessagesPrepended: session=" << sessionId
             << "olderCount=" << olderMessages.size() << "error=" << error;

    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);
    auto *page = sd->page;
    if(!page) return;

    sd->isLoadingMore = false;

    if(!error.isEmpty()){
        qDebug() << "[AIChatPane] Prepend messages error:" << error;
        return;
    }

    if(olderMessages.isEmpty()){
        sd->hasMoreMessages = false;
        qDebug() << "[AIChatPane] No older messages, hasMoreMessages set to false";
        return;
    }

    // Build only the older messages and prepend them in one insert batch.
    // A full resetMessages() here would destroy and re-create every delegate
    // (including visible ones) and reset the viewport, which both freezes
    // the UI on large histories and re-triggers the scroll-to-top load loop.
    QList<MsgInput> older;
    older.reserve(olderMessages.size());
    for (const auto &msg : olderMessages) {
        MsgInput mi;
        mi.type = (msg.role == "user")
            ? ChatMessageView::User
            : ChatMessageView::Assistant;
        mi.content = msg.text;
        mi.thinking = msg.thinking;
        older.append(mi);
    }

    qDebug() << "[AIChatPane] Prepending" << older.size() << "messages (existing:"
             << page->qmlModel()->rowCount() << ")";

    page->qmlModel()->prependMessages(older);

    // Update oldest timestamp
    sd->oldestMessageTimestamp = olderMessages.first().timeCreated;
    sd->hasMoreMessages = true;

    qDebug() << "[AIChatPane] Prepend complete, new oldestTimestamp=" << sd->oldestMessageTimestamp;
    // No explicit scroll here: MainChatView.qml re-anchors the previously
    // topmost message via aboutToPrependMessages (see QML Connections).
}

void AIChatPane::onLoadMoreMessages(const QString &sessionId)
{
    qDebug() << "[AIChatPane] onLoadMoreMessages called for session" << sessionId;

    int sdIdx = findSessionIndex(sessionId);
    if (sdIdx < 0) {
        qDebug() << "[AIChatPane] onLoadMoreMessages: session not found";
        return;
    }
    auto *sd = m_sessions.at(sdIdx);

    // Guard: don't fire while already loading, receiving, or when no more history
    if (sd->isLoadingMore || sd->isReceiving || !sd->hasMoreMessages || sd->oldestMessageTimestamp <= 0) {
        qDebug() << "[AIChatPane] LoadMore blocked: isLoadingMore=" << sd->isLoadingMore
                 << "isReceiving=" << sd->isReceiving
                 << "hasMore=" << sd->hasMoreMessages
                 << "oldestTs=" << sd->oldestMessageTimestamp;
        return;
    }

    qDebug() << "[AIChatPane] Loading older messages before:" << sd->oldestMessageTimestamp
             << "limit=20";
    sd->isLoadingMore = true;
    m_service->loadSessionMessages(sessionId, 20, sd->oldestMessageTimestamp);
    qDebug() << "[AIChatPane] loadSessionMessages called, isLoadingMore set to true";
}

// ---- message sending ----

void AIChatPane::onSendMessage(){
    auto *page = findPage(m_currentSessionId);
    if(!page) return;

    int sdIdx = findSessionIndex(m_currentSessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);

    if(sd->isReceiving) return;

    QString text = page->messageInput()->toPlainText().trimmed();
    if(text.isEmpty()) return;

    if(m_currentSessionId.isEmpty()){
        sd->pendingMessage = text;
        sd->firstUserMessage = text;
        page->messageInput()->clear();
        // Read model from page's combobox before creating session
        QString pid, mid;
        QString data = page->modelCombo()->currentData().toString();
        QStringList parts = data.split("::");
        if(parts.size() == 2){
            pid = parts[0];
            mid = parts[1];
        }
        m_service->createSession("", primaryWorkspacePath(), pid, mid);
        return;
    }

    renderUserMessage(page, text);
    page->messageInput()->clear();

    if(sd->firstUserMessage.isEmpty()){
        sd->firstUserMessage = text;
    }

    setSending(page, true);

    if(!m_service->sendMessage(m_currentSessionId, text, sd->modelProviderID, sd->modelID)){
        // request dropped because another request is still in flight
        setSending(page, false);
        appendEvent(page, tr("Previous request is still in progress, please retry"));
    }
}

// ---- response callbacks ----

void AIChatPane::onStreamStarted(const QString &sessionId){
    auto *page = findPage(sessionId);
    if(!page) return;

    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);

    sd->isReceiving = true;
    m_sessionPopup->updateSessionStatus(sessionId, true);

    if(sessionId == m_currentSessionId){
        setSending(page, true);
    }

    // Begin streaming via QML model (replaces ensureStreamingWidget)
    page->beginStreaming();
    sd->streamingRow = page->qmlModel()->messageCount() - 1;
    sd->isStreaming = true;

    // Create throttled scroll timer (once per session)
    if(!sd->scrollTimer){
        sd->scrollTimer = new QTimer(this);
        sd->scrollTimer->setInterval(300);
        sd->scrollTimer->setSingleShot(true);
        SessionPageWidget *p = page;
        connect(sd->scrollTimer, &QTimer::timeout, this, [this, p](){
            // Respect the user's scroll position: if they dragged away
            // from the bottom to read, don't yank the view back.
            p->autoFollowScroll();
        });
    }
}

void AIChatPane::onStreamChunk(const QString &sessionId, const QString &delta){
    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);
    if(!sd->page || !sd->isReceiving) return;

    sd->streamingContent.append(delta);
    sd->page->appendStreamingText(delta);
    throttledScrollToBottom(sd);
}

void AIChatPane::onStreamThinking(const QString &sessionId, const QString &content){
    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);
    if(!sd->page || !sd->isReceiving) return;

    sd->page->appendStreamingThinking(content);
    throttledScrollToBottom(sd);
}

void AIChatPane::onStreamToolUse(const QString &sessionId, const QString &callID, const QString &toolType, const QString &toolName, const QString &input){
    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);
    if(!sd->page || !sd->isReceiving) return;

    sd->page->appendToolCall(callID, toolType, toolName, input);
    throttledScrollToBottom(sd);
}

void AIChatPane::onStreamToolResult(const QString &sessionId, const QString &callID, const QString &toolType, const QString &toolName, const QString &output){
    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);
    if(!sd->page || !sd->isReceiving) return;

    // Determine success or failure based on output content
    int status = 1; // ToolSuccess
    if(output.contains("error", Qt::CaseInsensitive) || output.startsWith("Error:")) {
        status = 2; // ToolFailure
    }
    sd->page->updateToolCallStatus(callID, status, output);
    throttledScrollToBottom(sd);
}

void AIChatPane::onStreamFinished(const QString &sessionId, const QString &error){
    auto *page = findPage(sessionId);
    if(!page) return;

    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);

    // End streaming via QML model (replaces w->setStreaming(false))
    page->endStreaming();
    if(sd->scrollTimer) sd->scrollTimer->stop();

    // Always clear streaming bookkeeping first
    const bool wasReceiving = sd->isReceiving;
    const QString leftoverContent = sd->streamingContent;
    const int finishedRow = sd->streamingRow;
    sd->streamingRow = -1;
    sd->streamingContent.clear();
    sd->isStreaming = false;
    sd->isReceiving = false;
    sd->permissionPending = false;
    sd->permissionRow = -1;

    if(!wasReceiving) return;

    m_sessionPopup->updateSessionStatus(sessionId, false);

    if(!error.isEmpty()){
        if(!leftoverContent.isEmpty()){
            // Append error text to the existing streaming message
            page->appendStreamingText(tr("\n[error: %1]").arg(error));
        }else{
            // Remove empty streaming row if it exists
            if(finishedRow >= 0){
                page->qmlModel()->removeMessage(finishedRow);
            }
            appendEvent(page, tr("Error: %1").arg(error));
        }
    }

    if(sessionId == m_currentSessionId){
        setSending(page, false);
    }

    page->scrollToBottom();
}

void AIChatPane::onSessionStatusChanged(const QString &sessionId, const QString &status){
    auto *page = findPage(sessionId);
    if(!page) return;

    if(findSessionIndex(sessionId) < 0) return;

    if(status == "busy"){
        m_sessionPopup->updateSessionStatus(sessionId, true);
    }else if(status == "idle"){
        // isReceiving / streaming cleanup and send-button restore are handled
        // by onStreamFinished, which is always emitted right after this event.
        // Resetting isReceiving here would make onStreamFinished skip error
        // display and stale-row cleanup.
        m_sessionPopup->updateSessionStatus(sessionId, false);
    }
}

void AIChatPane::onSessionTitleChanged(const QString &sessionId, const QString &title){
    m_sessionPopup->updateSessionTitle(sessionId, title);
    auto *page = findPage(sessionId);
    if(page){
        page->sessionTitle()->setText(title.isEmpty() ? tr("New Chat") : title);
    }
}

// ---- model selection ----

void AIChatPane::onModelsReceived(const QList<OpenCodeModel> &models, const QString &error){
    if(!error.isEmpty()){
        qDebug() << "[AIChatPane] Load models failed:" << error;
        return;
    }

    m_modelsLoaded = true;

    updateAllModelCombos(models);
}

void AIChatPane::updateAllModelCombos(const QList<OpenCodeModel> &models)
{
    QString currentProvider = m_service->providerID();
    QString currentModel = m_service->modelID();
    QString defaultData = currentProvider + "/" + currentModel;

    for(auto *sd : m_sessions){
        QString selectedData = defaultData;
        if(!sd->modelProviderID.isEmpty() && !sd->modelID.isEmpty()){
            selectedData = sd->modelProviderID + "::" + sd->modelID;
        }
        sd->page->setModels(models, selectedData);
    }
}

// ---- session compaction ----

void AIChatPane::onCompactClicked(){
    auto *page = findPage(m_currentSessionId);
    if(!page){
        return;
    }
    int sdIdx = findSessionIndex(m_currentSessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);

    if(m_currentSessionId.isEmpty()){
        appendEvent(page, tr("No session selected"));
        return;
    }
    if(sd->isReceiving){
        appendEvent(page, tr("Cannot compact while receiving"));
        return;
    }
    qDebug() << "[AIChatPane] Compacting session:" << m_currentSessionId;
    appendEvent(page, tr("Compacting conversation..."));
    m_service->compactSession(m_currentSessionId);
}

void AIChatPane::onCompactionStarted(const QString &sessionId){
    qDebug() << "[AIChatPane] Compaction started for session:" << sessionId;
}

void AIChatPane::onCompactionFinished(const QString &sessionId){
    qDebug() << "[AIChatPane] Compaction finished for session:" << sessionId;
    auto *page = findPage(sessionId);
    if(!page) return;

    appendEvent(page, tr("Conversation compacted successfully"));
    m_service->loadSessionMessages(sessionId);
}

void AIChatPane::onConnectionChanged(bool connected)
{
    qDebug() << "[AIChatPane] Connection state:" << (connected ? "connected" : "disconnected");
    if(connected){
        // Server reconnected — reload sessions and models
        m_service->listSessions();
        if(!m_modelsLoaded){
            m_service->listModels();
        }
    }
}

void AIChatPane::onPermissionAsked(const OpenCodePermissionRequest &request)
{
    // Build description text
    QString detail;
    if(request.toolName == "bash" || request.toolName == "shell"){
        QJsonObject args = request.metadata.value("arguments").toObject();
        QString cmd = args.value("command").toString();
        if(cmd.isEmpty()) cmd = args.value("raw").toString();
        detail = tr("Command: %1").arg(cmd);
    }else if(request.toolName == "glob" || request.toolName == "read" || request.toolName == "write"){
        detail = tr("Patterns: %1").arg(request.patterns.join(", "));
    }else{
        detail = tr("Tool: %1").arg(request.toolName);
    }

    // Build JSON content for the permission widget
    QJsonObject permData;
    permData["requestId"] = request.id;
    permData["toolName"] = request.toolName;
    permData["detail"] = detail;
    QString jsonContent = QJsonDocument(permData).toJson(QJsonDocument::Compact);

    // Add permission message to chat area
    auto *page = findPage(request.sessionId);
    if(!page) page = findPage(m_currentSessionId);
    if(page){
        page->addMessage(ChatMessageView::Permission, jsonContent);
        page->scrollToBottom();

        // Mark permission as pending so streaming scroll pauses
        int sdIdx = findSessionIndex(request.sessionId);
        if(sdIdx < 0) sdIdx = findSessionIndex(m_currentSessionId);
        if(sdIdx >= 0){
            auto *sd = m_sessions.at(sdIdx);
            sd->permissionPending = true;
            sd->permissionRow = page->qmlModel()->messageCount() - 1;
            page->scrollToBottom();
        }
    }
}

void AIChatPane::onMemorySaved(const QString &sessionId, const QString &type,
                                const QString &content, const QString &keywords)
{
    // Type label mapping
    static QMap<QString, QString> typeLabels;
    if(typeLabels.isEmpty()){
        typeLabels["preference"] = tr("Preference");
        typeLabels["fact"] = tr("Fact");
        typeLabels["decision"] = tr("Decision");
        typeLabels["correction"] = tr("Correction");
        typeLabels["lesson"] = tr("Lesson");
    }

    QString label = typeLabels.value(type, type);
    QString text = QString("\xf0\x9f\x92\xbe ") + tr("Memory saved [%1]: %2").arg(label, content);

    auto *page = findPage(sessionId);
    if(!page) page = findPage(m_currentSessionId);
    if(page){
        page->addMessage(ChatMessageView::Event, text);
        page->scrollToBottom();
    }
}

// ---- rendering ----

void AIChatPane::renderUserMessage(SessionPageWidget *page, const QString &content){
    page->addMessage(ChatMessageView::User, content);
    page->scrollToBottom();
}

void AIChatPane::appendEvent(SessionPageWidget *page, const QString &text){
    page->addMessage(ChatMessageView::Event, text);
    page->scrollToBottom();
}

void AIChatPane::clearMessages(SessionPageWidget *page){
    page->clearMessages();
    for(auto *sd : m_sessions){
        if(sd->page == page){
            sd->streamingRow = -1;
            sd->streamingContent.clear();
            sd->isStreaming = false;
            break;
        }
    }
}

void AIChatPane::scrollToBottom(SessionPageWidget *page){
    page->scrollToBottom();
}

void AIChatPane::stopStreaming(const QString &sessionId)
{
    auto *page = findPage(sessionId);
    if(!page) return;

    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);

    // Stop streaming via QML model
    page->endStreaming();
    if(sd->scrollTimer) sd->scrollTimer->stop();

    sd->streamingRow = -1;
    sd->streamingContent.clear();
    sd->isStreaming = false;
    sd->isReceiving = false;

    m_sessionPopup->updateSessionStatus(sessionId, false);
    setSending(page, false);
}

void AIChatPane::throttledScrollToBottom(SessionData *sd)
{
    if(!sd->scrollTimer || !sd->page) return;
    // Pause auto-scroll when a permission request is pending — keep the
    // permission widget visible so the user can respond.
    if(sd->permissionPending) return;
    if(!sd->scrollTimer->isActive()){
        sd->scrollTimer->start();
    }
}

// ---- UI state ----

void AIChatPane::setSending(SessionPageWidget *page, bool sending){
    int sdIdx = -1;
    for(int i = 0; i < m_sessions.size(); ++i){
        if(m_sessions.at(i)->page == page){
            sdIdx = i;
            break;
        }
    }
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);

    sd->isReceiving = sending;

    if(sending){
        page->sendBtn()->setIcon(QIcon(":/Resource/icons/Stop_16x.svg"));
        page->sendBtn()->setToolTip(tr("Stop"));
    }else{
        page->sendBtn()->setIcon(QIcon(":/Resource/icons/TransferUpload_16x.svg"));
        page->sendBtn()->setToolTip(tr("Send (Ctrl+Enter)"));
    }


}

// ---- model preference persistence ----

QString AIChatPane::modelPreferencePath() const{
    return QCoreApplication::applicationDirPath() + "/data/ai_chat_model.json";
}

void AIChatPane::saveModelPreference(){
    QJsonObject root{
        {"providerID", m_service->providerID()},
        {"modelID", m_service->modelID()}
    };

    QString filePath = modelPreferencePath();
    QDir().mkpath(QFileInfo(filePath).absolutePath());
    QFile file(filePath);
    if(file.open(QIODevice::WriteOnly)){
        file.write(QJsonDocument(root).toJson());
    }
}

void AIChatPane::loadModelPreference(){
    QString filePath = modelPreferencePath();
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject obj = doc.object();
    m_service->setProviderID(obj["providerID"].toString());
    m_service->setModelID(obj["modelID"].toString());
}

// ---- opencode server auto-start (via terminal) ----

void AIChatPane::appendEventToCurrentPage(const QString &text)
{
    auto *page = findPage(m_currentSessionId);
    if(page){
        appendEvent(page, text);
    }
}

/**
 * opencode-cpp.exe 位于当前程序（IDE）所在目录
 */
QString AIChatPane::findServerExecutable()
{
    auto agentSetting = OptionsSettings::getInstance()->agentSettings();
    m_serverPort = agentSetting.m_port;
    m_service->setServerPort(m_serverPort);
    QString exeDir = QCoreApplication::applicationDirPath();
    QString exe = exeDir + "/openagent-cpp.exe";
    if(!QFile::exists(exe)){
        return QString();
    }

    // Ensure prompts/ directory exists next to the exe.
    // If missing, try to copy from the source tree (openagent-cpp/prompts/).
    QString promptsDir = exeDir + "/prompts";
    if(!QDir(promptsDir).exists()){
        // Try to find source prompts dir: walk up from exe to find openagent-cpp/prompts
        QString sourcePrompts;
        QDir dir(exeDir);
        for(int i = 0; i < 4; ++i){
            QString candidate = dir.filePath("openagent-cpp/prompts");
            if(QDir(candidate).exists()){
                sourcePrompts = candidate;
                break;
            }
            // Also check if this level itself has a prompts/ with default.txt
            candidate = dir.filePath("prompts");
            if(QDir(candidate).exists() && QFile::exists(candidate + "/default.txt")){
                sourcePrompts = candidate;
                break;
            }
            dir.cdUp();
        }
        if(!sourcePrompts.isEmpty()){
            qDebug() << "[AIChatPane] Copying prompts/ from:" << sourcePrompts << "to:" << promptsDir;
            QDir().mkpath(promptsDir);
            // Copy all files from source to dest
            for(const auto &entry : QDir(sourcePrompts).entryInfoList(QDir::Files)){
                QFile::copy(entry.filePath(), promptsDir + "/" + entry.fileName());
            }
        } else {
            qDebug() << "[AIChatPane] prompts/ directory not found next to exe or in source tree";
        }
    }

    return exe;
}

/**
 * listSessions 失败时调用：通过事件让终端打开并运行 openagent-cpp，
 * 然后链式 ping 等待服务器就绪，就绪后恢复正常的 API 加载流程。
 */
void AIChatPane::startServerIfNeeded()
{
    if(m_serverStartRequested) return;
    m_serverStartRequested = true;

    QString exe = findServerExecutable();
    if(exe.isEmpty()){
        qDebug() << "[AIChatPane] openagent-cpp executable not found";
        appendEventToCurrentPage(tr("openagent-cpp executable not found, "
                                    "please configure data/ai_chat_server.json"));
        return;
    }

    auto agentSetting = OptionsSettings::getInstance()->agentSettings();

    // Build command arguments
    QStringList args;
    args << "--port" << QString::number(m_serverPort);

    // Pass proxy if enabled in agent settings (reuse network proxy config)
    if(agentSetting.m_proxyEnabled){
        auto netSetting = OptionsSettings::getInstance()->networkSettings();
        if(!netSetting.m_host.isEmpty()){
            QString proxyUrl;
            if(!netSetting.m_username.isEmpty()){
                proxyUrl = QString("http://%1:%2@%3:%4")
                    .arg(netSetting.m_username, netSetting.m_password,
                         netSetting.m_host)
                    .arg(netSetting.m_port);
            } else {
                proxyUrl = QString("http://%1:%2")
                    .arg(netSetting.m_host).arg(netSetting.m_port);
            }
            args << "--proxy" << proxyUrl;
            qDebug() << "[AIChatPane] Passing proxy to openagent-cpp:" << proxyUrl;
        }
    }

    if(agentSetting.m_runMode == AgentSettings::Terminal){
        // Run in built-in terminal
        TerminalData td;
        td.workingDir = QFileInfo(exe).absolutePath();
        td.command = QString("\"%1\"").arg(exe);
        for(const auto &arg : args){
            td.command += QString(" %1").arg(arg);
        }
        Publisher::getInstance()->post(Type::M_OPEN_RUN_TERMINAL, &td);
        qDebug() << "[AIChatPane] Requested terminal to run:" << td.command;
        appendEventToCurrentPage(tr("Starting openagent-cpp in terminal..."));
    } else {
        // Run in QProcess (background)
        if(m_serverProcess){
            m_serverProcess->kill();
            m_serverProcess->deleteLater();
        }
        m_serverProcess = new QProcess(this);
        m_serverProcess->setWorkingDirectory(QFileInfo(exe).absolutePath());
        m_serverProcess->start(exe, args);
        qDebug() << "[AIChatPane] Started openagent-cpp in background:" << exe << args;
    }

    // chain-ping until the server responds
    m_pingCount = 0;
    m_service->pingServer();
}

void AIChatPane::onServerPingResult(bool ok)
{
    if(ok){
        qDebug() << "[AIChatPane] openagent-cpp server is ready";
        appendEventToCurrentPage(tr("openagent-cpp server is ready"));
        // server up - reload sessions (models follow in onSessionsReceived)
        m_service->listSessions();
        return;
    }

    m_pingCount++;
    if(m_pingCount >= 60){   // ~60 x (500ms + request) ≈ 60s
        qDebug() << "[AIChatPane] openagent-cpp server start timeout";
        appendEventToCurrentPage(tr("openagent-cpp start timeout, "
                                    "please check the terminal output"));
        return;
    }

    QTimer::singleShot(500, this, [this](){
        m_service->pingServer();
    });
}

// ---- workspace change notification ----

void AIChatPane::notifyWorkspacesChanged()
{
    QStringList paths;
    auto *model = ady::ResourceManagerModel::getInstance();
    if (model) {
        auto *root = model->rootItem();
        if (root) {
            for (int i = 0; i < root->childrenCount(); ++i) {
                auto *item = root->childAt(i);
                if (item && !item->path().isEmpty())
                    paths << item->path();
            }
        }
    }

    qDebug() << "[AIChatPane] workspaces changed, notifying openagent:" << paths;

    // Notify openagent-cpp server about working directory changes (global)
    m_service->setWorkingDirectories(paths);
}

// ---- workspace helper ----

QString AIChatPane::primaryWorkspacePath() const
{
    auto *model = ady::ResourceManagerModel::getInstance();
    if (!model) return {};
    auto *root = model->rootItem();
    if (!root || root->childrenCount() == 0) return {};
    auto *first = root->childAt(0);
    return first ? first->path() : QString();
}

QStringList AIChatPane::allWorkspacePaths() const
{
    QStringList paths;
    auto *model = ady::ResourceManagerModel::getInstance();
    if (!model) return paths;
    auto *root = model->rootItem();
    if (!root) return paths;
    for (int i = 0; i < root->childrenCount(); ++i) {
        auto *item = root->childAt(i);
        if (item && !item->path().isEmpty())
            paths << item->path();
    }
    return paths;
}

// ---- static factory methods ----

AIChatPane* AIChatPane::open(DockingPaneManager* dockingManager, bool active){
    if(instance == nullptr){
        instance = new AIChatPane(dockingManager->widget());
        DockingPaneLayoutItemInfo* item = dockingManager->createPane(instance, DockingPaneManager::S_Right, active);
        item->setManualSize(420);
    }
    return instance;
}

AIChatPane* AIChatPane::make(DockingPaneManager* dockingManager, const QJsonObject& /*data*/){
    if(instance == nullptr){
        instance = new AIChatPane(dockingManager->widget());
        return instance;
    }
    return nullptr;
}

AIChatPane* AIChatPane::getInstance(){
    return instance;
}

void AIChatPane::insertDebugTestMessage()
{
    auto *page = findPage(m_currentSessionId);
    if(!page) return;

    // Use the new QML streaming API to build a test message
    page->beginStreaming();

    // 1. Thinking content
    page->appendStreamingThinking("Let me analyze the user's request. They want to create a new file with some example code. "
                   "I should first check the existing project structure, then write the file using the write tool.");

    // 2. Tool call: read (processing)
    page->appendToolCall("call_debug_001", "read", "read", "{\"path\": \"D:/Qt/anycode/src/main.cpp\"}");

    // 3. Tool call: cmd (processing -> success)
    page->appendToolCall("call_debug_002", "cmd", "cmd", "{\"command\": \"dir D:\\Qt\\anycode\\src\"}");

    // 4. Tool call: write (processing -> success)
    page->appendToolCall("call_debug_003", "write", "write", "{\"path\": \"D:/Qt/anycode/src/test_output.cpp\"}");

    // Update read tool to success
    page->updateToolCallStatus("call_debug_001", 1,
                        "File content: #include <QApplication>\nint main(int argc, char *argv[]) {\n    QApplication app(argc, argv);\n    return app.exec();\n}");

    // Update cmd tool to success
    page->updateToolCallStatus("call_debug_002", 1,
                        " Volume in drive D is Local Disk\n Directory of D:\\Qt\\anycode\\src\n\n main.cpp\n test_output.cpp\n 2 File(s)");

    // Update write tool to success
    page->updateToolCallStatus("call_debug_003", 1,
                        "Wrote 256 bytes to D:/Qt/anycode/src/test_output.cpp");

    // 5. Summary text
    page->appendStreamingText("\n\nI've completed the task. Here's what I did:\n\n"
                  "1. **Read** the existing `main.cpp` to check the project structure\n"
                  "2. **Listed** files in the `src` directory using `dir` command\n"
                  "3. **Created** a new test file `test_output.cpp`\n\n"
                  "```cpp\n"
                  "#include <iostream>\n"
                  "#include <vector>\n\n"
                  "int main() {\n"
                  "    std::vector<int> nums = {1, 2, 3, 4, 5};\n"
                  "    for (auto n : nums) {\n"
                  "        std::cout << n << \" \";\n"
                  "    }\n"
                  "    return 0;\n"
                  "}\n"
                  "```\n\n"
                  "The file has been written successfully.");

    page->endStreaming();
    page->scrollToBottom();
}

}
