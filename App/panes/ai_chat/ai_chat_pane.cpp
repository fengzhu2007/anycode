#include "ai_chat_pane.h"
#include "ui_ai_chat_pane.h"
#include "chat_message_widget.h"
#include "components/message_dialog.h"
#include "message_list_view.h"
#include "message_model.h"
#include "session_list_popup.h"
#include "docking_pane_layout_item_info.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event.h"
#include "core/event_bus/event_data.h"
#include "core/theme.h"
#include "panes/resource_manager/resource_manager_model.h"
#include "panes/resource_manager/resource_manager_model_item.h"

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
#include <QCoreApplication>
#include <QDebug>
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
    connect(m_service, &ChatService::modelsReceived, this, &AIChatPane::onModelsReceived);
    connect(m_service, &ChatService::messagesReceived, this, &AIChatPane::onMessagesReceived);
    connect(m_service, &ChatService::compactionStarted, this, &AIChatPane::onCompactionStarted);
    connect(m_service, &ChatService::compactionFinished, this, &AIChatPane::onCompactionFinished);
    connect(m_service, &ChatService::autoCompactionTriggered, this, [this](const QString &sessionId){
        auto *page = findPage(sessionId);
        if(page){
            appendEvent(page, tr("Session exceeds 200KB, auto-compacting..."));
        }
    });
    connect(m_service, &ChatService::connectionChanged, this, &AIChatPane::onConnectionChanged);

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
    qDebug()<<"111:"<<id;
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
    }
}

void AIChatPane::onSessionsReceived(const QList<OpenCodeSession> &sessions, const QString &error){
    if(!error.isEmpty()){
        qDebug() << "[AIChatPane] Load sessions failed:" << error
                 << "- retrying in 5s";
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
        sd->modelProviderID = session.modelProviderID;
        sd->modelID = session.modelID;
        m_sessions.append(sd);
    }

    switchToSession(session.id);
    refreshSessionPopup();

    if(!sd->pendingMessage.isEmpty()){
        QString text = sd->pendingMessage;
        sd->pendingMessage.clear();

        renderUserMessage(page, text);
        setSending(page, true);

        // inject workspace directory context on first message of new session
        // (after renderUserMessage so UI shows clean text, only server receives context)
        if (sd->workspaceDirty) {
            QString ctx = buildWorkspaceContext();
            if (!ctx.isEmpty()) {
                text = ctx + text;
                sd->workspaceDirty = false;
            }
        }

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

    // Virtual list: restore streaming state when widgets are (re-)created
    connect(page->messageListView(), &MessageListView::widgetCreated,
            this, [this, page](int row, ChatMessageWidget *widget) {
        onWidgetCreated(page, row, widget);
    });

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
        // Copy model info from service's session list
        for(const auto &s : m_service->sessions()){
            if(s.id == sessionId){
                sd->modelProviderID = s.modelProviderID;
                sd->modelID = s.modelID;
                break;
            }
        }
        m_sessions.append(sd);

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
    if(!error.isEmpty()){
        qDebug() << "[AIChatPane] Load messages error:" << error;
        return;
    }

    auto *page = findPage(sessionId);
    if(!page) return;

    page->clearMessages();

    for(const auto &msg : messages){
        ChatMessageWidget::Type type = (msg.role == "user")
            ? ChatMessageWidget::User
            : ChatMessageWidget::Assistant;
        page->messageModel()->addMessage(type, msg.text);
    }

    page->scrollToBottom();
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

    // inject workspace directory context when workspace has changed
    if (sd->workspaceDirty) {
        QString ctx = buildWorkspaceContext();
        if (!ctx.isEmpty()) {
            text = ctx + text;
            sd->workspaceDirty = false;
        }
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

    ensureStreamingWidget(sd);
}

void AIChatPane::onStreamChunk(const QString &sessionId, const QString &delta){
    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);
    if(!sd->page || !sd->isReceiving) return;

    // IMPORTANT: ensureStreamingWidget uses streamingContent as the initial
    // content when creating the model row, so we must append AFTER widget
    // creation to avoid duplicating the first chunk.
    auto *w = ensureStreamingWidget(sd);
    sd->streamingContent.append(delta);
    if(w) w->appendText(delta);
    throttledScrollToBottom(sd);
}

void AIChatPane::onStreamThinking(const QString &sessionId, const QString &content){
    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);
    if(!sd->page || !sd->isReceiving) return;

    auto *w = ensureStreamingWidget(sd);
    if(w) w->appendThink(content);
    throttledScrollToBottom(sd);
}

void AIChatPane::onStreamToolUse(const QString &sessionId, const QString &toolName, const QString &input){
    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);
    if(!sd->page || !sd->isReceiving) return;

    auto *w = ensureStreamingWidget(sd);
    if(w) w->appendToolBlock(toolName, input, false);
    throttledScrollToBottom(sd);
}

void AIChatPane::onStreamToolResult(const QString &sessionId, const QString &toolName, const QString &output){
    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);
    if(!sd->page || !sd->isReceiving) return;

    auto *w = ensureStreamingWidget(sd);
    if(w) w->appendToolBlock(toolName, output, true);
    throttledScrollToBottom(sd);
}

void AIChatPane::onStreamFinished(const QString &sessionId, const QString &error){
    auto *page = findPage(sessionId);
    if(!page) return;

    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);

    // Stop spinner on visible widget (if any)
    const int finishedRow = sd->streamingRow;
    ChatMessageWidget *w = (finishedRow >= 0)
        ? page->messageListView()->widgetForMessage(finishedRow)
        : nullptr;
    if(w) w->setStreaming(false);
    if(sd->scrollTimer) sd->scrollTimer->stop();

    // Always clear streaming bookkeeping first — even when the user already
    // pressed Stop (isReceiving=false), the row must not be reused by the
    // next message, otherwise new chunks would append to the stale widget.
    const bool wasReceiving = sd->isReceiving;
    const QString leftoverContent = sd->streamingContent;
    sd->streamingRow = -1;
    sd->streamingContent.clear();
    sd->isStreaming = false;
    sd->isReceiving = false;

    if(!wasReceiving) return;

    m_sessionPopup->updateSessionStatus(sessionId, false);

    if(!error.isEmpty()){
        if(w && !leftoverContent.isEmpty()){
            w->appendText(tr("\n[error: %1]").arg(error));
        }else{
            // Remove empty streaming row if it exists
            if(finishedRow >= 0){
                page->messageModel()->removeMessage(finishedRow);
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

// ---- rendering ----

void AIChatPane::renderUserMessage(SessionPageWidget *page, const QString &content){
    page->addMessage(ChatMessageWidget::User, content);
    page->scrollToBottom();
}

void AIChatPane::appendEvent(SessionPageWidget *page, const QString &text){
    page->addMessage(ChatMessageWidget::Event, text);
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

ChatMessageWidget* AIChatPane::ensureStreamingWidget(SessionData *sd)
{
    SessionPageWidget *page = sd->page;
    auto *view = page->messageListView();
    auto *model = page->messageModel();

    // Create the streaming row if it doesn't exist yet
    if(sd->streamingRow < 0){
        sd->isStreaming = true;
        model->addMessage(ChatMessageWidget::Assistant, sd->streamingContent);
        sd->streamingRow = model->messageCount() - 1;

        // Force immediate widget creation for the new streaming row
        // (the deferred timer might not have fired yet)
        view->updateVisibleWidgets();

        // Create throttled scroll timer (once per session)
        if(!sd->scrollTimer){
            sd->scrollTimer = new QTimer(this);
            sd->scrollTimer->setInterval(300);
            sd->scrollTimer->setSingleShot(true);
            SessionPageWidget *p = page;
            connect(sd->scrollTimer, &QTimer::timeout, this, [this, p](){
                p->messageListView()->scrollToBottomImmediate();
            });
        }
    }

    // Try to get the widget from the visible viewport
    ChatMessageWidget *w = view->widgetForMessage(sd->streamingRow);
    if(w){
        w->setStreaming(true);
    }
    return w;  // may be nullptr if row is off-screen
}

void AIChatPane::stopStreaming(const QString &sessionId)
{
    auto *page = findPage(sessionId);
    if(!page) return;

    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);

    // Stop spinner immediately — do not wait for the server's idle event,
    // and make sure the row is never reused by the next message.
    ChatMessageWidget *w = (sd->streamingRow >= 0)
        ? page->messageListView()->widgetForMessage(sd->streamingRow)
        : nullptr;
    if(w) w->setStreaming(false);
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
    if(!sd->scrollTimer->isActive()){
        sd->scrollTimer->start();
    }
}

void AIChatPane::onWidgetCreated(SessionPageWidget *page, int row, ChatMessageWidget *widget)
{
    Q_UNUSED(page);
    // If this is the active streaming row, restore streaming indicator
    for(auto *sd : m_sessions){
        if(sd->page == page && sd->streamingRow == row && sd->isStreaming){
            widget->setStreaming(true);
            break;
        }
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

    qDebug() << "[AIChatPane] workspaces changed, notifying opencode:" << paths;

    // mark all sessions dirty so next message injects updated directory context
    for (auto *sd : m_sessions) {
        sd->workspaceDirty = true;
    }
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

QString AIChatPane::buildWorkspaceContext() const
{
    QStringList paths = allWorkspacePaths();
    // Filter out directories that no longer exist
    QStringList validPaths;
    for (const QString &p : paths) {
        if (QDir(p).exists())
            validPaths << p;
    }
    if (validPaths.isEmpty()) return {};
    if (validPaths.size() == 1)
        return "[Current working directory: " + validPaths.first() + "]\n";
    return "[Current working directories:\n" + validPaths.join("\n") + "]\n";
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

}
