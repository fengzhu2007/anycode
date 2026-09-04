#ifndef CHAT_MESSAGE_WIDGET_H
#define CHAT_MESSAGE_WIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>
#include <QTimer>

namespace Ui {
class ChatMessageWidget;
}

namespace ady{

/**
 * ChatMessageWidget - 单条聊天消息展示控件
 *
 * 支持多种消息类型(用户/助手/系统/错误/事件),
 * 助手消息支持简单的 markdown 代码块渲染,
 * 支持流式追加内容。
 */
class ChatMessageWidget : public QWidget
{
    Q_OBJECT
public:
    enum Type{
        User = 0,
        Assistant,
        System,
        Error,
        Event
    };

    explicit ChatMessageWidget(Type type, const QString &content, QWidget *parent = nullptr);
    ~ChatMessageWidget();

    Type type() const { return m_type; }
    QString content() const { return m_plainContent; }

    void setContent(const QString &text);
    void appendText(const QString &delta);

    /**
     * Set streaming state: shows animated loading indicator instead of timestamp.
     * Call setStreaming(true) when streaming starts, setStreaming(false) when done.
     */
    void setStreaming(bool streaming);

    /**
     * Append thinking content from SSE event (separate from text stream).
     * Content will be rendered as a collapsible block.
     */
    void appendThink(const QString &content);

    /**
     * Append a tool use/result block to the message.
     * @param toolName  display name of the tool
     * @param body      tool input or output content
     * @param isResult  true for tool_result, false for tool_use
     */
    void appendToolBlock(const QString &toolName, const QString &body, bool isResult = false);

    static QString roleOf(Type type);
    static Type typeOf(const QString &role);

    // QWidget overrides — public so MessageDelegate can query the
    // correct row height using the viewport width.
    QSize sizeHint() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int w) const override;

signals:
    /** Emitted after doUpdate() refreshes the rendered content.
     *  MessageListView uses this to trigger row-height recalculation. */
    void contentUpdated();

private slots:
    void onToggleThink();
    void onLinkActivated(const QString &link);

private:
    void paintEvent(QPaintEvent *event) override;
    void applyStyle();
    QString formatContent(const QString &text);
    QString extractAndFormatThinkContent(const QString &text, QString &cleaned) const;
    QString formatCodeBlock(const QString &code, int &outIndex);
    QString formatThinkBlock(const QString &thinkContent) const;
    void scheduleUpdate();
    void doUpdate();
    void updateSpinner();
    void copyCode(int index) const;

private:
    Ui::ChatMessageWidget *ui;
    Type m_type;
    QString m_plainContent;
    QString m_thinkContent;
    bool m_thinkExpanded = false;
    bool m_updateScheduled = false;
    bool m_streaming = false;
    bool m_thinkFromEvent = false;   // thinking arrived via SSE, skip tag extraction
    static int s_spinnerFrame;       // shared animation frame counter
    QTimer *m_spinnerTimer = nullptr; // drives spinner animation during streaming
    QStringList m_toolBlocks;        // pending tool block HTML (inserted via markers)
    QStringList m_codeBlocks;        // raw code text for clipboard copy
    QStringList m_codeHtml;          // code block HTML (inserted via CODE markers)
};

}

#endif // CHAT_MESSAGE_WIDGET_H
