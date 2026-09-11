#include "session_page_widget.h"
#include "ui_session_page_widget.h"
#include "message_list_view.h"
#include "message_model.h"
#include "chat_message_view.h"
#include "chat_service.h"
#include "core/theme.h"
#include <QKeyEvent>
#include <QTextCursor>
#include <QVBoxLayout>

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

    // Enter key handling for message input
    ui->messageInput->installEventFilter(this);
    ui->sessionTitle->setStyleSheet("QLabel{padding:4px}");
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

}
