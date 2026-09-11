#ifndef SESSION_PAGE_WIDGET_H
#define SESSION_PAGE_WIDGET_H

#include <QWidget>
#include <QList>
#include <QLabel>
#include "chat_message_view.h"

class QComboBox;
class QTextEdit;
class QToolButton;

namespace Ui {
class SessionPageWidget;
}

namespace ady {

struct OpenCodeModel;
class MessageListView;
class MessageModel;

/**
 * SessionPageWidget - 单个会话的聊天页面
 *
 * 每个会话包含独立的消息列表、模型选择、消息输入和发送按钮。
 * 作为 QStackedWidget 的一个 page 使用。
 */
class SessionPageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SessionPageWidget(QWidget *parent = nullptr);
    ~SessionPageWidget();

    MessageListView* messageListView() const;
    MessageModel* messageModel() const;
    QComboBox* modelCombo() const;
    QTextEdit* messageInput() const;
    QToolButton* sendBtn() const;
    QLabel* sessionTitle() const;

    void appendInputText(const QString &text);
    void setModels(const QList<OpenCodeModel> &models, const QString &selectedData);

    /** Convenience: add a message to the model and auto-scroll. */
    void addMessage(ChatMessageView::Type type, const QString &content);

    /** Convenience: clear all messages. */
    void clearMessages();

    /** Convenience: scroll to bottom (deferred). */
    void scrollToBottom();

signals:
    void enterPressed();
    void modelChanged(int index);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::SessionPageWidget *ui;
    MessageListView *m_messageListView = nullptr;
    MessageModel *m_messageModel = nullptr;
};

}

#endif // SESSION_PAGE_WIDGET_H
