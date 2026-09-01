#ifndef SESSION_LIST_POPUP_H
#define SESSION_LIST_POPUP_H

#include <QWidget>
#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPoint>
#include <QMouseEvent>
#include <QFocusEvent>
#include <functional>
#include "components/listview/listview_model.h"
#include "chat_service.h"

namespace ady {

/**
 * SessionListItemWidget - 会话列表项控件
 *
 * 单行显示：标题 | 状态图标 | 关闭按钮
 */
class SessionListItemWidget : public ListViewItem
{
    Q_OBJECT
public:
    explicit SessionListItemWidget(const QString &title, const QString &sessionId, QWidget *parent = nullptr);

    QString sessionId() const { return m_sessionId; }
    void setTitle(const QString &title);
    void setStatusIcon(bool busy);

signals:
    void clicked(const QString &sessionId);
    void closeClicked(const QString &sessionId);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    void handleClick();

    QString m_sessionId;
    QLabel *m_titleLabel;
    QLabel *m_statusIcon;
    QToolButton *m_closeBtn;
};


/**
 * SessionListModel - 会话列表数据模型
 *
 * 为 ListView 提供会话列表项。
 */
class SessionListModel : public ListViewModel
{
    Q_OBJECT
public:
    explicit SessionListModel(ListView *parent = nullptr);

    int count() override;
    ListViewItem* item(int i) override;
    ListViewItem* takeAt(int i) override;
    QWidget* emptyWidget() override;

    void refresh(const QList<OpenCodeSession> &sessions);
    void updateSessionTitle(const QString &sessionId, const QString &title);
    void updateSessionStatus(const QString &sessionId, bool busy);

    /**
     * 设置新项创建回调（用于连接项的信号）
     */
    void setItemCreatedCallback(std::function<void(SessionListItemWidget*)> callback);

private:
    QList<SessionListItemWidget*> m_items;
    QLabel *m_emptyWidget = nullptr;
    std::function<void(SessionListItemWidget*)> m_itemCreatedCallback;
};


/**
 * SessionListPopup - 会话列表弹窗
 *
 * 使用 ListView 组件显示会话列表，点击切换会话，支持关闭会话。
 */
class SessionListPopup : public QWidget
{
    Q_OBJECT
public:
    explicit SessionListPopup(QWidget *parent = nullptr);
    ~SessionListPopup();

    void refresh(const QList<OpenCodeSession> &sessions, const QString &currentSessionId);
    void updateSessionTitle(const QString &sessionId, const QString &title);
    void updateSessionStatus(const QString &sessionId, bool busy);
    void showAt(const QPoint &globalPos);

signals:
    void sessionClicked(const QString &sessionId);
    void sessionCloseClicked(const QString &sessionId);

protected:
    void focusOutEvent(QFocusEvent *event) override;

private:
    QScrollArea *m_scrollArea;
    QWidget *m_container;
    QVBoxLayout *m_itemsLayout;
    QList<SessionListItemWidget*> m_items;
};

}

#endif // SESSION_LIST_POPUP_H
