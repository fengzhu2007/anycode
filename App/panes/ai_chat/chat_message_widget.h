#ifndef CHAT_MESSAGE_WIDGET_H
#define CHAT_MESSAGE_WIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>

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

    static QString roleOf(Type type);
    static Type typeOf(const QString &role);

private slots:
    void onToggleThink();

private:
    void paintEvent(QPaintEvent *event) override;
    void applyStyle();
    QString formatContent(const QString &text) const;
    QString extractAndFormatThinkContent(const QString &text, QString &cleaned) const;
    QString formatCodeBlock(const QString &code) const;
    void scheduleUpdate();
    void doUpdate();

private:
    Ui::ChatMessageWidget *ui;
    Type m_type;
    QString m_plainContent;
    QString m_thinkContent;
    bool m_thinkExpanded = false;
    bool m_updateScheduled = false;
};

}

#endif // CHAT_MESSAGE_WIDGET_H
