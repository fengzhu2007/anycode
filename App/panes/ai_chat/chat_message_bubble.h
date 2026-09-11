#ifndef CHAT_MESSAGE_BUBBLE_H
#define CHAT_MESSAGE_BUBBLE_H

#include <QWidget>
#include <QPaintEvent>
#include <QTimer>
#include <QUrl>
#include <QStringList>

class QLabel;
class QTextBrowser;
class QVBoxLayout;
class QResizeEvent;

namespace ady{

/**
 * ChatMessageBubble - 单条聊天消息展示控件(QTextDocument 渲染版)
 *
 * 与旧版 ChatMessageWidget(QLabel 实现)公开接口完全一致,可互换使用。
 * 通过 chat_message_view.h 中的 ADY_USE_CHAT_MESSAGE_BUBBLE 宏切换。
 *
 * 与旧版的主要区别:
 *  - 内容渲染在持久的 QTextDocument 上(经 QTextBrowser 显示),高度计算
 *    直接使用 document()->setTextWidth() + document()->size(),不依赖
 *    QLabel 内部的富文本路径;
 *  - 支持完整 markdown:标题、加粗、斜体、行内代码、删除线、无序/有序/
 *    任务列表、链接、图片(转为链接)、引用块、分隔线;围栏代码块仍带
 *    语言标签与复制按钮;
 *  - 流式 spinner 是独立的 QLabel,动画刷新不再触发整篇文档重渲染;
 *  - 文本可选中,QTextBrowser 原生支持右键菜单复制。
 */
class ChatMessageBubble : public QWidget
{
    Q_OBJECT
public:
    enum Type{
        User = 0,
        Assistant,
        System,
        Error,
        Event,
        Permission
    };

    enum ToolStatus {
        ToolProcessing = 0,
        ToolSuccess,
        ToolFailure
    };

    struct ToolCallInfo {
        QString callID;
        QString toolType;       // raw tool identifier from SSE: "cmd", "read", "write" etc.
        QString toolName;       // display name (may include input summary)
        ToolStatus status = ToolProcessing;
        QString body;           // raw input/output text
        bool isCommand = false; // command-line tool (bash/cmd/powershell)
        QString shellType;      // "cmd", "powershell", "shell"
        int blockIndex = -1;    // index in m_toolBlocks
    };

    explicit ChatMessageBubble(Type type, const QString &content, QWidget *parent = nullptr);
    ~ChatMessageBubble();

    Type type() const { return m_type; }
    QString content() const { return m_plainContent; }

    void setContent(const QString &text);
    void appendText(const QString &delta);

    /**
     * Set streaming state: shows/hides the standalone spinner row.
     * Spinner ticks only update the QLabel text — the document is NOT
     * re-rendered on animation frames.
     */
    void setStreaming(bool streaming);

    /**
     * Append thinking content from SSE event (separate from text stream).
     * Content will be rendered as a collapsible block.
     */
    void appendThink(const QString &content);

    /**
     * Append a tool use block to the message with Processing status.
     * @param callID    unique call identifier for matching with tool_result
     * @param toolName  display name of the tool
     * @param body      tool input content (command or parameters)
     */
    void appendToolBlock(const QString &callID, const QString &toolType, const QString &toolName, const QString &body);

    /**
     * Update an existing tool block's status by callID.
     * Called when tool_result is received.
     * @param callID    the call identifier to match
     * @param status    Success or Failure
     * @param output    tool output content (optional, may be empty)
     */
    void updateToolStatus(const QString &callID, ToolStatus status, const QString &output = QString());

    /**
     * Set permission request data (for Permission type).
     * Parses JSON content to extract requestId, toolName, detail.
     * Creates action buttons (Allow Once / Allow Always / Reject).
     */
    void setupPermissionUI();

    /**
     * Mark permission as replied: remove buttons, append chosen action.
     * Used when widget is recreated after scroll virtualization.
     */
    void setPermissionReplied(const QString &reply);

    static QString roleOf(Type type);
    static Type typeOf(const QString &role);

    // QWidget overrides — public so MessageDelegate can query the
    // correct row height using the viewport width.
    QSize sizeHint() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int w) const override;

signals:
    /** Emitted after doUpdate() renders changed content.
     *  MessageListView uses this to trigger row-height recalculation. */
    void contentUpdated();

    /** Emitted when user clicks a permission action button.
     *  @param requestId  permission request ID
     *  @param reply      "once", "always", or "reject" */
    void permissionReplied(const QString &requestId, const QString &reply);

private slots:
    void onToggleThink();
    void onAnchorClicked(const QUrl &url);

private:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    /**
     * Pin the document's wrap width to the browser's actual display width.
     * QTextEdit re-wraps at its viewport width only when its own width
     * changes; a width pinned by a transient heightForWidth() measurement
     * would otherwise stick and render the content in a narrow column.
     */
    void syncDocumentWidth();

    /**
     * Apply a proportional line height (150%) to every block after each
     * setHtml(): the generated HTML is mostly bare text + <br/> lines whose
     * implicit blocks cannot be targeted by a CSS rule, so the line height
     * is set directly on each QTextBlockFormat instead.
     */
    void applyLineHeight();

    void buildUi();
    void applyStyle();
    QString formatContent(const QString &text);
    QString extractAndFormatThinkContent(const QString &text, QString &cleaned) const;
    QString formatThinkBlock(const QString &thinkContent) const;
    QString formatCodeBlock(const QString &code);
    void scheduleUpdate();
    void doUpdate();
    void updateSpinner();
    void copyCode(int index) const;
    QString buildToolBlockHtml(const ToolCallInfo &info) const;
    static QString detectShellType(const QString &toolName, const QString &command);

private:
    Type m_type;
    QVBoxLayout *m_mainLayout = nullptr;
    QLabel *m_spinnerLabel = nullptr;     // 流式指示器(与文档渲染完全解耦)
    QTextBrowser *m_browser = nullptr;    // 持久 QTextDocument 的内容视图
    QWidget *m_buttonRow = nullptr;       // Permission 按钮行(可空)

    QString m_plainContent;
    QString m_lastHtml;                   // 上次渲染的 HTML,内容未变化时跳过重渲染
    QString m_thinkContent;
    bool m_thinkExpanded = false;
    bool m_updateScheduled = false;
    bool m_streaming = false;
    bool m_thinkFromEvent = false;        // thinking 来自 SSE 事件,跳过标签提取
    QTimer *m_spinnerTimer = nullptr;     // 驱动 spinner 动画
    QStringList m_toolBlocks;             // tool 块 HTML(通过标记插入)
    QList<ToolCallInfo> m_toolCalls;      // tool 调用跟踪(按 callID 匹配更新)
    QStringList m_codeBlocks;             // 代码块原文(供复制按钮使用)

    // Permission request state
    QString m_permissionRequestId;   // request ID for reply callback
    QString m_permissionReply;       // non-empty after user has replied
};

}

#endif // CHAT_MESSAGE_BUBBLE_H
