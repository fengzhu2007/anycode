#ifndef SESSION_PAGE_WIDGET_H
#define SESSION_PAGE_WIDGET_H

#include <QWidget>
#include <QList>
#include <QLabel>
#include "chat_message_view.h"

class QComboBox;
class QTextEdit;
class QToolButton;
class QQuickWidget;
class wPopupPanel;

namespace Ui {
class SessionPageWidget;
}

namespace ady {

struct OpenCodeModel;
struct FileDiffInfo;
class QmlMessageModel;
class FileDiffListWidget;
class ChatService;

/**
 * SessionPageWidget - 单个会话的聊天页面
 *
 * 每个会话包含独立的消息列表、模型选择、消息输入和发送按钮。
 * 作为 QStackedWidget 的一个 page 使用。
 *
 * 使用 QmlMessageModel + QQuickWidget + ListView 虚拟化渲染。
 */
class SessionPageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SessionPageWidget(QWidget *parent = nullptr);
    ~SessionPageWidget();

    // ---- QML 接口 ----
    QmlMessageModel* qmlModel() const { return m_qmlModel; }
    QQuickWidget*    quickWidget() const { return m_quickWidget; }

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

    // ---- 通用便捷方法 (自动路由到 QML 模型) ----

    /** Add a message to the model and auto-scroll. */
    void addMessage(ChatMessageView::Type type, const QString &content);

    /** Clear all messages. */
    void clearMessages();

    /** Scroll to bottom (deferred). */
    void scrollToBottom();

    /** Streaming follow-up: scroll to bottom only if the user hasn't
     *  dragged away (delegates to MainChatView.autoFollow). */
    void autoFollowScroll();

    // ---- QML 流式 API ----

    /** Begin a new streaming assistant message. */
    void beginStreaming();

    /** Append text delta to the streaming message. */
    void appendStreamingText(const QString &delta);

    /** Append/replace thinking content on the streaming message. */
    void appendStreamingThinking(const QString &content);

    /** Append or update a tool call on the streaming message. */
    void appendToolCall(const QString &callID, const QString &toolType,
                        const QString &toolName, const QString &input);

    /** Update tool call status by callID. */
    void updateToolCallStatus(const QString &callID, int status, const QString &output);

    /** Mark streaming as finished. */
    void endStreaming();

signals:
    void enterPressed();
    void modelChanged(int index);
    void scrollToTopRequested();
    void fileOpenRequested(const QString &filePath);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::SessionPageWidget *ui;

    // QML 消息列表
    QmlMessageModel *m_qmlModel = nullptr;
    QQuickWidget    *m_quickWidget = nullptr;

    ChatService *m_chatService = nullptr;
    QString m_sessionId;

    wPopupPanel *m_diffPopup = nullptr;
    FileDiffListWidget *m_diffListWidget = nullptr;
    QList<FileDiffInfo> m_currentDiffs;

    void setupDiffPopup();
    void setupQmlView();
    void onAcceptAll();
    void onRejectAll();
};

}

#endif // SESSION_PAGE_WIDGET_H
