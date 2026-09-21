#include "session_page_widget.h"
#include "ui_session_page_widget.h"
#include "qml_message_model.h"
#include "chat_service.h"
#include "file_diff_list_widget.h"
#include "session_config_dialog.h"
#include "core/theme.h"
#include "w_popup_panel.h"
#include <QKeyEvent>
#include <QTextCursor>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QTimer>
#include <QQuickWidget>
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlError>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>

namespace ady {

SessionPageWidget::SessionPageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SessionPageWidget)
{
    ui->setupUi(this);

    // ---- QML 消息列表 ----
    setupQmlView();

    // Enter key handling for message input
    ui->messageInput->installEventFilter(this);
    ui->fileChanged->installEventFilter(this);
    ui->sessionTitle->setStyleSheet("QLabel{padding:4px}");

    // Session config button
    ui->sessionConfigBtn->setCursor(Qt::PointingHandCursor);
    ui->sessionConfigBtn->setStyleSheet(
        "QToolButton{padding:4px;border:none;}"
        "QToolButton:hover{background:rgba(128,128,128,40);border-radius:4px;}");
    connect(ui->sessionConfigBtn, &QToolButton::clicked,
            this, &SessionPageWidget::onSessionConfigClicked);

    // File diff popup
    setupDiffPopup();

    // --- TEST DEMO: inject 10 fake file changes ---
   /* {
        QList<FileDiffInfo> testDiffs;
        QStringList statuses = {"modified", "added", "deleted"};
        for (int i = 0; i < 10; ++i) {
            FileDiffInfo d;
            d.file = QString("D:/wamp/www/oa5/src/test_file_%1.cpp").arg(i + 1);
            d.status = statuses[i % 3];
            d.additions = (i + 1) * 3;
            d.deletions = (i + 1);
            testDiffs.append(d);
        }
        setFileDiffs(testDiffs);
    }*/
}

SessionPageWidget::~SessionPageWidget()
{
    delete ui;
}

QComboBox* SessionPageWidget::modelCombo() const { return ui->modelCombo; }
QTextEdit* SessionPageWidget::messageInput() const { return ui->messageInput; }
QToolButton* SessionPageWidget::sendBtn() const { return ui->sendBtn; }
QLabel* SessionPageWidget::sessionTitle() const { return ui->sessionTitle; }

void SessionPageWidget::appendInputText(const QString &text)
{
    ui->messageInput->moveCursor(QTextCursor::End);
    ui->messageInput->insertPlainText(text);
    ui->messageInput->setFocus();
}

void SessionPageWidget::setModels(const QList<OpenCodeModel> &models, const QString &selectedData)
{
    ui->modelCombo->blockSignals(true);
    ui->modelCombo->clear();

    for(const auto &m : models){
        QString displayName = m.name.isEmpty() ? m.modelID : m.name;
        QString data = m.providerID + "::" + m.modelID;
        ui->modelCombo->addItem(displayName, data);
    }

    if(!selectedData.isEmpty()){
        int idx = ui->modelCombo->findData(selectedData);
        if(idx >= 0){
            ui->modelCombo->setCurrentIndex(idx);
        }
    }
    ui->modelCombo->blockSignals(false);
}

bool SessionPageWidget::eventFilter(QObject *obj, QEvent *event)
{
    // fileChanged label click -> toggle popup
    if (obj == ui->fileChanged && event->type() == QEvent::MouseButtonPress) {
        if (!m_currentDiffs.isEmpty()) {
            if (m_diffPopup->isVisible()) {
                m_diffPopup->hide();
            } else {
                m_diffPopup->showPopup(ui->fileChanged, wPopupPanel::TopLeft);
            }
        }
        return true;
    }

    if (obj == ui->messageInput && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (keyEvent->modifiers() & Qt::AltModifier) {
                ui->messageInput->insertPlainText("\n");
                return true;
            } else {
                emit enterPressed();
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

// ---- File diff popup ----

void SessionPageWidget::setupDiffPopup()
{
    // Create popup panel
    m_diffPopup = new wPopupPanel(this);
    m_diffPopup->setBorderRadius(8);
    m_diffPopup->setContentPadding(0);
    m_diffPopup->setBackgroundColor(QColor(30, 30, 30, 240));
    m_diffPopup->setAutoClose(true);

    // Create file diff list widget
    m_diffListWidget = new FileDiffListWidget;
    m_diffPopup->setContentWidget(m_diffListWidget);

    // Connect file diff list signals
    connect(m_diffListWidget, &FileDiffListWidget::acceptAll, this, &SessionPageWidget::onAcceptAll);
    connect(m_diffListWidget, &FileDiffListWidget::rejectAll, this, &SessionPageWidget::onRejectAll);

    // fileChanged label click -> show popup
    ui->fileChanged->setCursor(Qt::PointingHandCursor);

    // Hide bottom bar initially (no changes yet)
    ui->fileChanged->hide();
    ui->accept->hide();
    ui->reject->hide();

    // Accept/Reject buttons
    connect(ui->accept, &QToolButton::clicked, this, &SessionPageWidget::onAcceptAll);
    connect(ui->reject, &QToolButton::clicked, this, &SessionPageWidget::onRejectAll);
}

void SessionPageWidget::setChatService(ChatService *service)
{
    m_chatService = service;
}

void SessionPageWidget::setSessionId(const QString &sessionId)
{
    m_sessionId = sessionId;
}

void SessionPageWidget::setSessionPreference(const QString &preference)
{
    m_sessionPreference = preference;
}

QString SessionPageWidget::sessionPreference() const
{
    return m_sessionPreference;
}

void SessionPageWidget::setSessionDirectory(const QString &dir)
{
    m_sessionDir = dir;
}

QString SessionPageWidget::sessionDirectory() const
{
    return m_sessionDir;
}

void SessionPageWidget::setDirectoryList(const QStringList &paths)
{
    m_dirList = paths;
}

void SessionPageWidget::setFileDiffs(const QList<FileDiffInfo> &diffs)
{
    m_currentDiffs = diffs;

    bool hasDiffs = !diffs.isEmpty();
    ui->fileChanged->setVisible(hasDiffs);
    ui->accept->setVisible(hasDiffs);
    ui->reject->setVisible(hasDiffs);

    if (hasDiffs) {
        ui->fileChanged->setText(tr("File Changed List (%1)").arg(diffs.size()));
        m_diffListWidget->setDiffs(diffs);
    } else {
        m_diffListWidget->clear();
        if (m_diffPopup->isVisible()) {
            m_diffPopup->hide();
        }
    }
}

void SessionPageWidget::onAcceptAll()
{
    if (!m_chatService || m_sessionId.isEmpty()) return;
    qDebug() << "[SessionPage] Accept all changes for session:" << m_sessionId;
    m_chatService->confirmChanges(m_sessionId);
    m_currentDiffs.clear();
    m_diffListWidget->clear();
    ui->fileChanged->hide();
    ui->accept->hide();
    ui->reject->hide();
    if (m_diffPopup->isVisible()) {
        m_diffPopup->hide();
    }
}

void SessionPageWidget::onRejectAll()
{
    if (!m_chatService || m_sessionId.isEmpty()) return;
    qDebug() << "[SessionPage] Reject all changes for session:" << m_sessionId;
    m_chatService->revertSession(m_sessionId);
    m_currentDiffs.clear();
    m_diffListWidget->clear();
    ui->fileChanged->hide();
    ui->accept->hide();
    ui->reject->hide();
    if (m_diffPopup->isVisible()) {
        m_diffPopup->hide();
    }
}

// ---- QML view setup ----

void SessionPageWidget::setupQmlView()
{
    // Create QML message model
    m_qmlModel = new QmlMessageModel(this);

    // Create QQuickWidget
    m_quickWidget = new QQuickWidget(this);
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    //m_quickWidget->setFocusPolicy(Qt::NoFocus);

    // Match QQuickWidget background to theme
    QColor bgColor = Theme::getInstance()->backgroundColor();
    m_quickWidget->setStyleSheet("background-color: " + bgColor.name(QColor::HexRgb) + ";");

    // Expose model and theme color to QML
    m_quickWidget->rootContext()->setContextProperty("messageModel", m_qmlModel);
    m_quickWidget->rootContext()->setContextProperty("themeBgColor", bgColor.name(QColor::HexRgb));

    // TEST MODE: render fixed-height stub rows instead of real message
    // bodies (no parts, no markdown TextEdit, no model content access).
    // Used to isolate the scroll-freeze cause — if the freeze disappears
    // with this enabled, the problem lives in the rendering pipeline
    // (MarkdownBody/TextEdit), otherwise in the view/model layer.
    // Set to false to restore normal rendering after diagnosis.
    m_quickWidget->rootContext()->setContextProperty("messageTestMode", false);

    // Load main QML
    m_quickWidget->setSource(QUrl("qrc:/ai_chat/qml/MainChatView.qml"));

    // Log QML load errors to file
    //qDebug()<<"qml error:"<<m_quickWidget->errors();
    //logQmlErrors(m_quickWidget->errors());

    // Connect runtime QML warnings to log file
    /*connect(m_quickWidget->engine(), &QQmlEngine::warnings,
            this, [this](const QList<QQmlError> &errors) {
        logQmlErrors(errors);
    });*/
    // Place QQuickWidget inside the messageAreaWidget container
    auto *msgLayout = new QVBoxLayout(ui->messageAreaWidget);
    msgLayout->setContentsMargins(0, 0, 0, 0);
    msgLayout->setSpacing(0);
    msgLayout->addWidget(m_quickWidget);

    // Forward scroll-to-bottom requests from QML
    connect(m_qmlModel, &QmlMessageModel::scrollToBottomRequested, this, [this]() {
        // The QML ListView handles its own scrolling; this is for C++ callers
    });

    // Forward permission replies from QML parts
    connect(m_qmlModel, &QmlMessageModel::permissionReplied,
            this, [this](const QString &requestId, const QString &reply) {
        if (m_chatService)
            m_chatService->replyPermission(requestId, reply);
        // NOTE: do NOT emit scrollToTopRequested here — the pane connects
        // that signal to onLoadMoreMessages(), which would reload history
        // and rebuild the model right after the user clicks a permission
        // button (destroying the card under the cursor).
    });

    // Forward file open requests from QML parts
    connect(m_qmlModel, &QmlMessageModel::fileOpenRequested,
            this, [this](const QString &filePath) {
        emit fileOpenRequested(filePath);
    });
}

// ---- QML error logging ----

void SessionPageWidget::logQmlErrors(const QList<QQmlError> &errors)
{
    if (errors.isEmpty())
        return;

    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(logDir);
    QString logPath = logDir + "/qml_errors.log";

    QFile file(logPath);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "[SessionPage] Failed to open QML error log:" << logPath;
        return;
    }

    QTextStream out(&file);
    out << "\n========== " << QDateTime::currentDateTime().toString(Qt::ISODate) << " ==========\n";
    for (const auto &err : errors) {
        out << err.toString() << "\n";
    }
    out << "========================================\n";
    file.close();

    // Also print to debug output
    for (const auto &err : errors) {
        qWarning() << "[QML Error]" << err.toString();
    }
}

// ---- convenience methods (route to QML model) ----

void SessionPageWidget::addMessage(ChatMessageView::Type type, const QString &content)
{
    m_qmlModel->addMessage(type, content);
}

void SessionPageWidget::todoUpdated(const QString &todoListId, const QString &taskId,
                                    const QString &status, const QString &output)
{
    m_qmlModel->todoUpdated(todoListId, taskId, status, output);
}

void SessionPageWidget::clearMessages()
{
    m_qmlModel->clearMessages();
}

void SessionPageWidget::scrollToBottom()
{
    // Call QML function via the root object
    if (m_quickWidget && m_quickWidget->rootObject()) {
        QMetaObject::invokeMethod(static_cast<QObject*>(m_quickWidget->rootObject()), "scrollToBottom");
    }
}

void SessionPageWidget::autoFollowScroll()
{
    // Streaming follow-up: QML only scrolls when the user hasn't dragged
    // away from the bottom (autoScroll flag in MainChatView.qml)
    if (m_quickWidget && m_quickWidget->rootObject()) {
        QMetaObject::invokeMethod(static_cast<QObject*>(m_quickWidget->rootObject()), "autoFollow");
    }
}

// ---- QML streaming API ----

void SessionPageWidget::beginStreaming()
{
    m_qmlModel->beginStreaming();
}

void SessionPageWidget::appendStreamingText(const QString &delta)
{
    m_qmlModel->appendStreamingText(delta);
}

void SessionPageWidget::appendStreamingThinking(const QString &content)
{
    m_qmlModel->appendStreamingThinking(content);
}

void SessionPageWidget::appendToolCall(const QString &callID, const QString &toolType,
                                        const QString &toolName, const QString &input)
{
    m_qmlModel->appendToolCall(callID, toolType, toolName, input);
}

void SessionPageWidget::updateToolCallStatus(const QString &callID, int status, const QString &output)
{
    m_qmlModel->updateToolCallStatus(callID, status, output);
}

void SessionPageWidget::endStreaming()
{
    m_qmlModel->endStreaming();
}

// ---- Session config dialog ----

void SessionPageWidget::onSessionConfigClicked()
{
    if (!m_configDialog) {
        m_configDialog = new SessionConfigDialog(this);
    }
    m_configDialog->setSessionTitle(ui->sessionTitle->text());
    m_configDialog->setPreference(m_sessionPreference);
    m_configDialog->setDirectory(m_sessionDir);
    m_configDialog->setDirectoryList(m_dirList);

    if (m_configDialog->exec() == QDialog::Accepted) {
        const QString newTitle = m_configDialog->sessionTitle();
        const QString newPref = m_configDialog->preference();
        const QString newDir = m_configDialog->directory();

        // Update local state
        ui->sessionTitle->setText(newTitle);
        m_sessionPreference = newPref;
        m_sessionDir = newDir;

        // Update session via API: title, directory, preference
        if (m_chatService && !m_sessionId.isEmpty()) {
            m_chatService->updateSession(m_sessionId, newTitle, newDir, newPref);
        }

        // Notify parent (AIChatPane) for further handling
        emit sessionConfigApplied(m_sessionId, newTitle, newPref, newDir);
    }
}

}
