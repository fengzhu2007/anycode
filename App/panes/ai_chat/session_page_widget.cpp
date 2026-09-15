#include "session_page_widget.h"
#include "ui_session_page_widget.h"
#include "message_list_view.h"
#include "message_model.h"
#include "chat_message_view.h"
#include "chat_service.h"
#include "file_diff_list_widget.h"
#include "core/theme.h"
#include "w_popup_panel.h"
#include <QKeyEvent>
#include <QTextCursor>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QTimer>

namespace ady {

SessionPageWidget::SessionPageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SessionPageWidget)
{
    ui->setupUi(this);

    // Create virtualized message list (replaces the old QScrollArea + QVBoxLayout)
    m_messageModel = new MessageModel(this);
    m_messageListView = new MessageListView(this);
    m_messageListView->setMessageModel(m_messageModel);

    // Place MessageListView inside the messageAreaWidget container
    auto *msgLayout = new QVBoxLayout(ui->messageAreaWidget);
    msgLayout->setContentsMargins(0, 0, 0, 0);
    msgLayout->setSpacing(0);
    msgLayout->addWidget(m_messageListView);

    // Forward scroll-to-top signal for loading older messages
    connect(m_messageListView, &MessageListView::scrollToTopRequested,
            this, &SessionPageWidget::scrollToTopRequested);

    // Enter key handling for message input
    ui->messageInput->installEventFilter(this);
    ui->fileChanged->installEventFilter(this);
    ui->sessionTitle->setStyleSheet("QLabel{padding:4px}");

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

MessageListView* SessionPageWidget::messageListView() const { return m_messageListView; }
MessageModel* SessionPageWidget::messageModel() const { return m_messageModel; }
QComboBox* SessionPageWidget::modelCombo() const { return ui->modelCombo; }
QTextEdit* SessionPageWidget::messageInput() const { return ui->messageInput; }
QToolButton* SessionPageWidget::sendBtn() const { return ui->sendBtn; }
QLabel* SessionPageWidget::sessionTitle() const { return ui->sessionTitle; }

void SessionPageWidget::addMessage(ChatMessageView::Type type, const QString &content)
{
    m_messageModel->addMessage(type, content);
}

void SessionPageWidget::clearMessages()
{
    m_messageModel->clearMessages();
}

void SessionPageWidget::scrollToBottom()
{
    m_messageListView->scrollToBottomDeferred();
}

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
    connect(ui->accept, &QPushButton::clicked, this, &SessionPageWidget::onAcceptAll);
    connect(ui->reject, &QPushButton::clicked, this, &SessionPageWidget::onRejectAll);
}

void SessionPageWidget::setChatService(ChatService *service)
{
    m_chatService = service;
}

void SessionPageWidget::setSessionId(const QString &sessionId)
{
    m_sessionId = sessionId;
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

}
