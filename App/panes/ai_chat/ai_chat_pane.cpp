#include "ai_chat_pane.h"
#include "ui_ai_chat_pane.h"
#include "chat_message_widget.h"
#include "docking_pane_layout_item_info.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event.h"
#include "core/theme.h"

#include <QAction>
#include <QScrollBar>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

namespace ady{

AIChatPane* AIChatPane::instance = nullptr;

const QString AIChatPane::PANE_ID = "AIChat";
const QString AIChatPane::PANE_GROUP = "AIChat";

class AIChatPanePrivate{
public:
    QString currentSessionId;
    bool isReceiving = false;
    ChatMessageWidget* streamingWidget = nullptr;   // 当前流式输出的消息控件
    QString pendingMessage;                          // 等待会话创建完成后发送的消息
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
    QString bgColor = Theme::getInstance()->backgroundColor().name(QColor::HexRgb);
    QString borderColor = Theme::getInstance()->borderColor().name(QColor::HexRgb);
    QString secBgColor = Theme::getInstance()->secondaryBackgroundColor().name(QColor::HexRgb);
    this->setStyleSheet(
        "QToolBar{border:0px;}"
        "QScrollArea#messageScroll{border:0;background-color:" + bgColor + ";}"
        "QScrollArea#messageScroll>QWidget>QWidget{border:0;background-color:" + bgColor + ";}"
        "QTextEdit{border:1px solid " + borderColor + ";}"
        "QListWidget{border:0;border-right:1px solid " + borderColor + ";background-color:" + secBgColor + ";}"
        "QSplitter::handle{background-color:" + borderColor + ";}"
        "QLabel#statusLabel{color:#888;padding:2px;}"
    );

    d = new AIChatPanePrivate;

    // create chat service
    m_service = new ChatService(this);

    // toolbar – add send / stop actions
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionSend);
    ui->toolBar->addAction(ui->actionStop);
    ui->actionStop->setEnabled(false);

    // connections
    connect(ui->actionNewChat, &QAction::triggered, this, &AIChatPane::onActionTriggered);
    connect(ui->actionDeleteChat, &QAction::triggered, this, &AIChatPane::onActionTriggered);
    connect(ui->actionClear, &QAction::triggered, this, &AIChatPane::onActionTriggered);
    connect(ui->actionRefreshModels, &QAction::triggered, this, &AIChatPane::onActionTriggered);
    connect(ui->actionSend, &QAction::triggered, this, &AIChatPane::onSendMessage);
    connect(ui->actionStop, &QAction::triggered, this, [this](){
        if(!d->currentSessionId.isEmpty()){
            m_service->abortSession(d->currentSessionId);
        }
        setSending(false);
    });
    connect(ui->sendBtn, &QPushButton::clicked, this, &AIChatPane::onSendMessage);
    connect(ui->sessionList, &QListWidget::currentRowChanged, this, [this](int){ onSessionSelected(); });

    // model selection
    connect(ui->modelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AIChatPane::onModelChanged);

    // service signals
    connect(m_service, &ChatService::sessionsReceived, this, &AIChatPane::onSessionsReceived);
    connect(m_service, &ChatService::sessionCreated, this, &AIChatPane::onSessionCreated);
    connect(m_service, &ChatService::sessionDeleted, this, &AIChatPane::onSessionDeleted);
    connect(m_service, &ChatService::streamStarted, this, &AIChatPane::onStreamStarted);
    connect(m_service, &ChatService::streamChunk, this, &AIChatPane::onStreamChunk);
    connect(m_service, &ChatService::streamFinished, this, &AIChatPane::onStreamFinished);
    connect(m_service, &ChatService::sessionStatusChanged, this, &AIChatPane::onSessionStatusChanged);
    connect(m_service, &ChatService::modelsReceived, this, &AIChatPane::onModelsReceived);

    // splitter initial sizes (session list ~180px, chat fills the rest)
    ui->splitter->setSizes({180, 470});

    Subscriber::reg();
    this->initView();
}

AIChatPane::~AIChatPane(){
    Subscriber::unReg();
    saveModelPreference();
    instance = nullptr;

    // 先删除 service，确保所有网络连接关闭
    delete m_service;
    m_service = nullptr;

    delete d;
    delete ui;
}

void AIChatPane::initView(){
    // input font
    QFont font("Consolas", 10);
    font.setStyleHint(QFont::Monospace);
    ui->messageInput->setFont(font);

    // load model preference
    loadModelPreference();

    // load sessions from server first, then listModels will be called in onSessionsReceived
    loadSessionsFromServer();
}

// ---- DockingPane overrides ----

QString AIChatPane::id(){
    return AIChatPane::PANE_ID;
}

QString AIChatPane::group(){
    return AIChatPane::PANE_GROUP;
}

bool AIChatPane::onReceive(Event* /*e*/){
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
        m_service->createSession();
    } else if(sender == ui->actionDeleteChat){
        int row = ui->sessionList->currentRow();
        if(row >= 0){
            auto item = ui->sessionList->item(row);
            QString id = item->data(Qt::UserRole).toString();
            m_service->deleteSession(id);
        }
    } else if(sender == ui->actionClear){
        clearMessages();
    } else if(sender == ui->actionRefreshModels){
        m_service->listModels();
    }
}

void AIChatPane::onSessionSelected(){
    int row = ui->sessionList->currentRow();
    if(row < 0) return;

    auto item = ui->sessionList->item(row);
    QString id = item->data(Qt::UserRole).toString();
    d->currentSessionId = id;
    m_service->setCurrentSessionId(id);

    // 清空当前消息显示
    clearMessages();

    // TODO: 从服务器加载该会话的历史消息
    // 目前 opencode API 没有提供获取会话历史消息的端点
    // 消息通过 SSE 事件流实时接收

    scrollToBottom();
}

void AIChatPane::onSessionsReceived(const QList<OpenCodeSession> &sessions, const QString &error){
    if(!error.isEmpty()){
        ui->statusLabel->setText(tr("Load sessions failed: %1").arg(error));
        return;
    }

    refreshSessionList();

    // 自动选中最近的会话
    if(ui->sessionList->count() > 0 && d->currentSessionId.isEmpty()){
        ui->sessionList->setCurrentRow(0);
    }

    // now load models after sessions are loaded
    m_service->listModels();
}

void AIChatPane::onSessionCreated(const OpenCodeSession &session, const QString &error){
    if(!error.isEmpty()){
        ui->statusLabel->setText(tr("Create session failed: %1").arg(error));
        d->pendingMessage.clear();
        return;
    }

    refreshSessionList();

    // 选中新创建的会话
    for(int i = 0; i < ui->sessionList->count(); ++i){
        if(ui->sessionList->item(i)->data(Qt::UserRole).toString() == session.id){
            ui->sessionList->setCurrentRow(i);
            break;
        }
    }

    // 如果有待发送的消息，自动发送
    if(!d->pendingMessage.isEmpty()){
        QString text = d->pendingMessage;
        d->pendingMessage.clear();
        
        renderUserMessage(text);
        setSending(true);
        m_service->sendMessage(session.id, text);
    }
}

void AIChatPane::onSessionDeleted(const QString &sessionId, const QString &error){
    if(!error.isEmpty()){
        ui->statusLabel->setText(tr("Delete session failed: %1").arg(error));
        return;
    }

    if(d->currentSessionId == sessionId){
        d->currentSessionId.clear();
        clearMessages();
    }

    refreshSessionList();

    // 选中其他会话
    if(ui->sessionList->count() > 0){
        ui->sessionList->setCurrentRow(0);
    }
}

void AIChatPane::refreshSessionList(){
    ui->sessionList->clear();
    auto sessions = m_service->sessions();
    for(const auto &s : sessions){
        QString title = s.title.isEmpty() ? tr("New Chat") : s.title;
        QListWidgetItem *item = new QListWidgetItem(title);
        item->setData(Qt::UserRole, s.id);
        item->setToolTip(title);
        ui->sessionList->addItem(item);
    }
}

void AIChatPane::loadSessionsFromServer(){
    m_service->listSessions();
}

// ---- message sending ----

void AIChatPane::onSendMessage(){
    if(d->isReceiving) return;

    QString text = ui->messageInput->toPlainText().trimmed();
    if(text.isEmpty()) return;

    // 确保有会话
    if(d->currentSessionId.isEmpty()){
        // 保存待发送消息，等会话创建完成后自动发送
        d->pendingMessage = text;
        ui->messageInput->clear();
        m_service->createSession();
        return;
    }

    // 渲染用户消息
    renderUserMessage(text);
    ui->messageInput->clear();
    setSending(true);

    // 发送消息
    m_service->sendMessage(d->currentSessionId, text);
}

// ---- response callbacks ----

void AIChatPane::onStreamStarted(const QString &sessionId){
    if(sessionId != d->currentSessionId) return;
    d->isReceiving = true;
    ui->statusLabel->setText(tr("Receiving..."));

    // create the streaming assistant message widget
    d->streamingWidget = new ChatMessageWidget(ChatMessageWidget::Assistant, {}, ui->messageContainer);
    ui->messageLayout->insertWidget(ui->messageLayout->count() - 1, d->streamingWidget);
    scrollToBottom();
}

void AIChatPane::onStreamChunk(const QString &sessionId, const QString &delta){
    if(sessionId != d->currentSessionId || d->streamingWidget == nullptr) return;
    d->streamingWidget->appendText(delta);
    scrollToBottom();
}

void AIChatPane::onStreamFinished(const QString &sessionId, const QString &error){
    if(sessionId != d->currentSessionId) return;
    if(!d->isReceiving) return;  // 防止重复触发
    d->isReceiving = false;

    if(!error.isEmpty()){
        if(d->streamingWidget && !d->streamingWidget->content().isEmpty()){
            d->streamingWidget->appendText(tr("\n[error: %1]").arg(error));
        }else{
            renderUserMessage("");  // placeholder
            appendEvent(tr("Error: %1").arg(error));
        }
    }
    d->streamingWidget = nullptr;

    ui->statusLabel->clear();
    setSending(false);
    scrollToBottom();
}

void AIChatPane::onSessionStatusChanged(const QString &sessionId, const QString &status){
    if(sessionId != d->currentSessionId) return;

    if(status == "busy"){
        ui->statusLabel->setText(tr("AI is thinking..."));
    }else if(status == "idle"){
        ui->statusLabel->clear();
        setSending(false);
    }
}

// ---- model selection ----

void AIChatPane::onModelsReceived(const QList<OpenCodeModel> &models, const QString &error){
    if(!error.isEmpty()){
        ui->statusLabel->setText(tr("Load models failed: %1").arg(error));
        return;
    }

    // 记住当前选择
    QString currentProvider = m_service->providerID();
    QString currentModel = m_service->modelID();

    // block signals while rebuilding the list
    ui->modelCombo->blockSignals(true);
    ui->modelCombo->clear();

    // 添加模型到下拉列表，UserRole 存储 "providerID/modelID"
    for(const auto &m : models){
        QString displayName = m.name.isEmpty() ? m.modelID : m.name;
        QString data = m.providerID + "/" + m.modelID;
        ui->modelCombo->addItem(displayName, data);
    }

    // 恢复之前的选择
    if(!currentProvider.isEmpty() && !currentModel.isEmpty()){
        QString data = currentProvider + "/" + currentModel;
        int idx = ui->modelCombo->findData(data);
        if(idx >= 0){
            ui->modelCombo->setCurrentIndex(idx);
        }
    }
    ui->modelCombo->blockSignals(false);

    // 应用选择
    if(ui->modelCombo->count() > 0){
        int idx = ui->modelCombo->currentIndex();
        onModelChanged(idx);
        appendEvent(tr("Models loaded (%1)").arg(models.size()));
    }
    ui->statusLabel->clear();
}

void AIChatPane::onModelChanged(int index){
    if(index < 0) return;
    QString data = ui->modelCombo->itemData(index).toString();
    QStringList parts = data.split("/");
    if(parts.size() == 2){
        m_service->setProviderID(parts[0]);
        m_service->setModelID(parts[1]);
        saveModelPreference();
        appendEvent(tr("Switched to: %1").arg(ui->modelCombo->currentText()));
    }
}

// ---- rendering ----

void AIChatPane::renderUserMessage(const QString &content){
    auto widget = new ChatMessageWidget(ChatMessageWidget::User, content, ui->messageContainer);
    ui->messageLayout->insertWidget(ui->messageLayout->count() - 1, widget);
    scrollToBottom();
}

void AIChatPane::appendEvent(const QString &text){
    auto widget = new ChatMessageWidget(ChatMessageWidget::Event, text, ui->messageContainer);
    ui->messageLayout->insertWidget(ui->messageLayout->count() - 1, widget);
    scrollToBottom();
}

void AIChatPane::clearMessages(){
    auto layout = ui->messageLayout;
    QLayoutItem *item;
    while((item = layout->takeAt(0)) != nullptr){
        if(item->widget()){
            delete item->widget();
        }
        delete item;
    }
    // re-add the bottom spacer so messages stay top-aligned
    layout->addStretch(1);
    d->streamingWidget = nullptr;
}

void AIChatPane::scrollToBottom(){
    auto bar = ui->messageScroll->verticalScrollBar();
    bar->setValue(bar->maximum());
}

// ---- UI state ----

void AIChatPane::setSending(bool sending){
    d->isReceiving = sending;
    ui->sendBtn->setEnabled(!sending);
    ui->actionSend->setEnabled(!sending);
    ui->actionStop->setEnabled(sending);
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
