#include "ai_chat_pane.h"
#include "ui_ai_chat_pane.h"
#include "chat_message_widget.h"
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
#include <QScrollArea>
#include <QVBoxLayout>
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

    // toolbar – add send / stop / compact actions
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionSend);
    ui->toolBar->addAction(ui->actionStop);
    ui->toolBar->addAction(ui->actionCompact);
    ui->actionStop->setEnabled(false);

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
    connect(ui->actionClear, &QAction::triggered, this, &AIChatPane::onActionTriggered);
    connect(ui->actionRefreshModels, &QAction::triggered, this, &AIChatPane::onActionTriggered);
    connect(ui->actionCompact, &QAction::triggered, this, &AIChatPane::onCompactClicked);
    connect(ui->actionSend, &QAction::triggered, this, &AIChatPane::onSendMessage);
    connect(ui->actionSessions, &QAction::triggered, [this](){
        refreshSessionPopup();
        QPoint topLeft = ui->stackedWidget->mapToGlobal(QPoint(0, 0));
        int width = ui->stackedWidget->width();
        m_sessionPopup->setFixedWidth(width);
        m_sessionPopup->showAt(topLeft);
    });
    connect(ui->actionStop, &QAction::triggered, [this](){
        if(!m_currentSessionId.isEmpty()){
            m_service->abortSession(m_currentSessionId);
        }
        auto *page = findPage(m_currentSessionId);
        if(page) setSending(page, false);
    });

    // service signals
    connect(m_service, &ChatService::sessionsReceived, this, &AIChatPane::onSessionsReceived);
    connect(m_service, &ChatService::sessionCreated, this, &AIChatPane::onSessionCreated);
    connect(m_service, &ChatService::sessionDeleted, this, &AIChatPane::onSessionDeleted);
    connect(m_service, &ChatService::streamStarted, this, &AIChatPane::onStreamStarted);
    connect(m_service, &ChatService::streamChunk, this, &AIChatPane::onStreamChunk);
    connect(m_service, &ChatService::streamFinished, this, &AIChatPane::onStreamFinished);
    connect(m_service, &ChatService::sessionStatusChanged, this, &AIChatPane::onSessionStatusChanged);
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

    Subscriber::reg();
    this->regMessageIds({Type::M_OPEN_PROJECT, Type::M_CLOSE_PROJECT,
                         Type::M_ADD_TO_CHAT, Type::M_ADD_TO_NEW_CHAT});

    // workspace change debounce timer: 2s delay after last project open/close
    m_workspaceNotifyTimer = new QTimer(this);
    m_workspaceNotifyTimer->setSingleShot(true);
    m_workspaceNotifyTimer->setInterval(2000);
    connect(m_workspaceNotifyTimer, &QTimer::timeout, this, &AIChatPane::notifyWorkspacesChanged);

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
            m_service->createSession("", primaryWorkspacePath());
            page->appendInputText(text);
        }else{
            auto *page = findPage(m_currentSessionId);
            if(page){
                page->appendInputText(text);
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
        m_service->createSession("", primaryWorkspacePath());
    } else if(sender == ui->actionDeleteChat){
        if(!m_currentSessionId.isEmpty()){
            m_service->deleteSession(m_currentSessionId);
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
        qDebug() << "[AIChatPane] Load sessions failed:" << error;
        return;
    }

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
                QStringList parts = data.split("/");
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
        m_service->sendMessage(session.id, text, sd->modelProviderID, sd->modelID);
    }
}

void AIChatPane::onSessionDeleted(const QString &sessionId, const QString &error){
    if(!error.isEmpty()){
        qDebug() << "[AIChatPane] Delete session failed:" << error;
        return;
    }

    int idx = findSessionIndex(sessionId);
    if(idx < 0) return;

    auto *sd = m_sessions.at(idx);

    if(m_currentSessionId == sessionId){
        m_currentSessionId.clear();
    }

    removePage(sd->page);
    m_sessions.removeAt(idx);
    delete sd;

    refreshSessionPopup();

    if(!m_sessions.isEmpty()){
        switchToSession(m_sessions.first()->sessionId);
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
            setSending(page, false);
        }else{
            onSendMessage();
        }
    });

    connect(page->modelCombo(), QOverload<int>::of(&QComboBox::currentIndexChanged), [this, page](int index){
        if(index < 0) return;
        if(page != findPage(m_currentSessionId)) return;
        QString data = page->modelCombo()->itemData(index).toString();
        QStringList parts = data.split("/");
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
        QString defaultData = m_service->providerID() + "/" + m_service->modelID();
        page->setModels(models, defaultData);
    }

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
            QString modelData = sd->modelProviderID + "/" + sd->modelID;
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

    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);

    const QList<OpenCodeMessage> &sorted = messages;

    if(sd->firstUserMessage.isEmpty()){
        for(const auto &msg : sorted){
            if(msg.role == "user" && !msg.text.isEmpty()){
                sd->firstUserMessage = msg.text;
                break;
            }
        }
    }

    if(!sd->firstUserMessage.isEmpty()){
        m_sessionPopup->updateSessionTitle(sessionId, sd->firstUserMessage.left(50));
        QString title = sd->firstUserMessage;
        if(title.length() > 50) title = title.left(50) + "...";
        m_service->updateSessionTitle(sessionId, title);
        QTimer::singleShot(500, this, [this](){
            m_service->listSessions();
        });
    }

    clearMessages(page);

    for(const auto &msg : sorted){
        ChatMessageWidget::Type type = (msg.role == "user")
            ? ChatMessageWidget::User
            : ChatMessageWidget::Assistant;
        auto *w = new ChatMessageWidget(type, msg.text, page->messageScrollContents());
        page->messageContainer()->insertWidget(page->messageContainer()->count() - 1, w);
    }

    scrollToBottom(page);
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
        m_service->createSession("", primaryWorkspacePath());
        return;
    }

    renderUserMessage(page, text);
    page->messageInput()->clear();

    if(sd->firstUserMessage.isEmpty()){
        sd->firstUserMessage = text;
    }

    setSending(page, true);

    m_service->sendMessage(m_currentSessionId, text, sd->modelProviderID, sd->modelID);
}

// ---- response callbacks ----

void AIChatPane::onStreamStarted(const QString &sessionId){
    qDebug() << "[AIChatPane] onStreamStarted, sessionId=" << sessionId;

    auto *page = findPage(sessionId);
    if(!page){
        qDebug() << "[AIChatPane] onStreamStarted: no page for sessionId";
        return;
    }

    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);

    sd->isReceiving = true;

    m_sessionPopup->updateSessionStatus(sessionId, true);

    if(sessionId == m_currentSessionId){
        setSending(page, true);
    }

    // create the streaming assistant message widget
    sd->streamingWidget = new ChatMessageWidget(ChatMessageWidget::Assistant, {}, page->messageScrollContents());
    page->messageContainer()->insertWidget(page->messageContainer()->count() - 1, sd->streamingWidget);
}

void AIChatPane::onStreamChunk(const QString &sessionId, const QString &delta){
    qDebug() << "[AIChatPane] onStreamChunk, sessionId=" << sessionId
             << "deltaLen=" << delta.length();

    auto *page = findPage(sessionId);
    if(!page) return;

    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);

    // Auto-create streaming widget if not yet created (lazy init)
    if(sd->streamingWidget == nullptr){
        sd->isReceiving = true;
        sd->streamingWidget = new ChatMessageWidget(ChatMessageWidget::Assistant, {}, page->messageScrollContents());
        page->messageContainer()->insertWidget(page->messageContainer()->count() - 1, sd->streamingWidget);
    }

    sd->streamingWidget->appendText(delta);
}

void AIChatPane::onStreamFinished(const QString &sessionId, const QString &error){
    qDebug() << "[AIChatPane] onStreamFinished, sessionId=" << sessionId
             << "error=" << error;

    auto *page = findPage(sessionId);
    if(!page) return;

    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);

    if(!sd->isReceiving) return;
    sd->isReceiving = false;

    m_sessionPopup->updateSessionStatus(sessionId, false);

    if(!error.isEmpty()){
        if(sd->streamingWidget && !sd->streamingWidget->content().isEmpty()){
            sd->streamingWidget->appendText(tr("\n[error: %1]").arg(error));
        }else{
            if(sd->streamingWidget){
                delete sd->streamingWidget;
                sd->streamingWidget = nullptr;
            }
            appendEvent(page, tr("Error: %1").arg(error));
        }
    }
    sd->streamingWidget = nullptr;

    if(sessionId == m_currentSessionId){
        setSending(page, false);
    }

    if(!sd->firstUserMessage.isEmpty()){
        QString title = sd->firstUserMessage;
        if(title.length() > 50){
            title = title.left(50) + "...";
        }
        m_service->updateSessionTitle(sessionId, title);
        QTimer::singleShot(500, this, [this](){
            m_service->listSessions();
        });
    }

    QMetaObject::invokeMethod(this, [this, page](){ scrollToBottom(page); }, Qt::QueuedConnection);
}

void AIChatPane::onSessionStatusChanged(const QString &sessionId, const QString &status){
    auto *page = findPage(sessionId);
    if(!page) return;

    int sdIdx = findSessionIndex(sessionId);
    if(sdIdx < 0) return;
    auto *sd = m_sessions.at(sdIdx);

    if(status == "busy"){
        qDebug() << "[AIChatPane] Session busy:" << sessionId;
        m_sessionPopup->updateSessionStatus(sessionId, true);
    }else if(status == "idle"){
        sd->isReceiving = false;
        m_sessionPopup->updateSessionStatus(sessionId, false);

        if(sessionId == m_currentSessionId){
            setSending(page, false);
        }

        if(!sd->firstUserMessage.isEmpty()){
            QString title = sd->firstUserMessage;
            if(title.length() > 50){
                title = title.left(50) + "...";
            }
            m_service->updateSessionTitle(sessionId, title);
            sd->firstUserMessage.clear();
            QTimer::singleShot(500, this, [this](){
                m_service->listSessions();
            });
        }
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
            selectedData = sd->modelProviderID + "/" + sd->modelID;
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

// ---- rendering ----

void AIChatPane::renderUserMessage(SessionPageWidget *page, const QString &content){
    auto widget = new ChatMessageWidget(ChatMessageWidget::User, content, page->messageScrollContents());
    page->messageContainer()->insertWidget(page->messageContainer()->count() - 1, widget);
    scrollToBottom(page);
}

void AIChatPane::appendEvent(SessionPageWidget *page, const QString &text){
    auto widget = new ChatMessageWidget(ChatMessageWidget::Event, text, page->messageScrollContents());
    page->messageContainer()->insertWidget(page->messageContainer()->count() - 1, widget);
    scrollToBottom(page);
}

void AIChatPane::clearMessages(SessionPageWidget *page){
    auto layout = page->messageContainer();
    QLayoutItem *item;
    while((item = layout->takeAt(0)) != nullptr){
        if(item->widget()){
            delete item->widget();
        }
        delete item;
    }
    // re-add the bottom spacer so messages stay top-aligned
    layout->addStretch(1);

    for(auto *sd : m_sessions){
        if(sd->page == page){
            sd->streamingWidget = nullptr;
            break;
        }
    }
}

void AIChatPane::scrollToBottom(SessionPageWidget *page){
    QScrollBar *sb = page->messageScrollArea()->verticalScrollBar();
    if(sb) sb->setValue(sb->maximum());
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

    if(page == findPage(m_currentSessionId)){
        ui->actionSend->setEnabled(!sending);
        ui->actionStop->setEnabled(sending);
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
