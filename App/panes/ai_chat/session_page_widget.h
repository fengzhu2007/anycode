#ifndef SESSION_PAGE_WIDGET_H
#define SESSION_PAGE_WIDGET_H

#include <QWidget>
#include <QList>
#include <QLabel>
#include "chat_message_view.h"

class QComboBox;
class QTextEdit;
class QToolButton;
class wPopupPanel;

namespace Ui {
class SessionPageWidget;
}

namespace ady {

struct OpenCodeModel;
struct FileDiffInfo;
class MessageListView;
class MessageModel;
class FileDiffListWidget;
class ChatService;

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

    /** 设置 ChatService 用于 API 调用 */
    void setChatService(ChatService *service);

    /** 设置当前会话 ID */
    void setSessionId(const QString &sessionId);

    /** 更新文件变更列表 */
    void setFileDiffs(const QList<FileDiffInfo> &diffs);

    /** Convenience: add a message to the model and auto-scroll. */
    void addMessage(ChatMessageView::Type type, const QString &content);

    /** Convenience: clear all messages. */
    void clearMessages();

    /** Convenience: scroll to bottom (deferred). */
    void scrollToBottom();

signals:
    void enterPressed();
    void modelChanged(int index);
    void scrollToTopRequested();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::SessionPageWidget *ui;
    MessageListView *m_messageListView = nullptr;
    MessageModel *m_messageModel = nullptr;

    ChatService *m_chatService = nullptr;
    QString m_sessionId;

    wPopupPanel *m_diffPopup = nullptr;
    FileDiffListWidget *m_diffListWidget = nullptr;
    QList<FileDiffInfo> m_currentDiffs;

    void setupDiffPopup();
    void onAcceptAll();
    void onRejectAll();
};

}

#endif // SESSION_PAGE_WIDGET_H
