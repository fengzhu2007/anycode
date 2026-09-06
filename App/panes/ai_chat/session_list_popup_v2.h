#ifndef SESSION_LIST_POPUP_V2_H
#define SESSION_LIST_POPUP_V2_H

#include <QWidget>
#include <QListView>
#include <QAbstractListModel>
#include <QStyledItemDelegate>
#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPoint>
#include <QFocusEvent>
#include <QTimer>
#include "chat_service.h"

namespace ady {

// Forward declaration - reuse existing item widget
class SessionListItemWidget;

// ============================================================================
// SessionListModelV2 - QAbstractListModel for session data
// ============================================================================

class SessionListModelV2 : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit SessionListModelV2(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setSessions(const QList<OpenCodeSession> &sessions);
    void updateTitle(const QString &sessionId, const QString &title);
    void updateStatus(const QString &sessionId, bool busy);

    OpenCodeSession sessionAt(int row) const;

private:
    QList<OpenCodeSession> m_sessions;
};

// ============================================================================
// SessionListDelegateV2 - minimal delegate, defers rendering to index widgets
// ============================================================================

class SessionListDelegateV2 : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SessionListDelegateV2(QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

// ============================================================================
// SessionListPopupV2 - QListView-based session list popup with cascade animation
// ============================================================================

class SessionListPopupV2 : public QWidget
{
    Q_OBJECT
public:
    explicit SessionListPopupV2(QWidget *parent = nullptr);
    ~SessionListPopupV2();

    void refresh(const QList<OpenCodeSession> &sessions, const QString &currentSessionId);
    void updateSessionTitle(const QString &sessionId, const QString &title);
    void updateSessionStatus(const QString &sessionId, bool busy);
    void showAt(const QPoint &globalPos);

signals:
    void sessionClicked(const QString &sessionId);
    void sessionCloseClicked(const QString &sessionId);

protected:
    void focusOutEvent(QFocusEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void ensureIndexWidgets();
    void startCascadeAnimation();
    void startCloseAnimation();

    QListView *m_listView;
    SessionListModelV2 *m_model;
    SessionListDelegateV2 *m_delegate;
    QLabel *m_emptyLabel;
    QList<OpenCodeSession> m_pendingSessions;
    QList<SessionListItemWidget*> m_indexWidgets;
    QPoint m_popupPos;
    bool m_animScheduled;
};

}

#endif // SESSION_LIST_POPUP_V2_H
