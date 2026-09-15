#ifndef MESSAGE_LIST_VIEW_H
#define MESSAGE_LIST_VIEW_H

/**
 * @file message_list_view.h
 * @brief Virtualized QListView for chat messages.
 *
 * Only creates ChatMessageView (bubble) instances for rows visible in the
 * viewport. Widgets for off-screen rows are destroyed to save memory.
 * A small buffer of widgets above/below the viewport is kept for smooth scrolling.
 */

#include <QListView>
#include <QHash>
#include "chat_message_view.h"

class QTimer;

namespace ady {

class MessageModel;

/**
 * MessageListView - virtualized chat message list.
 *
 * Lifecycle:
 *  - When a row enters the visible viewport (+ buffer), a ChatMessageView
 *    is created via setIndexWidget() and populated from model data.
 *  - When a row leaves the viewport (+ buffer), the widget is destroyed.
 *  - For streaming rows, the widget can be retrieved via widgetForMessage().
 *    If the streaming row scrolled off-screen, the widget returns nullptr
 *    and the caller must re-create it when the row becomes visible again.
 */
class MessageListView : public QListView
{
    Q_OBJECT
public:
    explicit MessageListView(QWidget *parent = nullptr);

    /** Set the backing model (must be a MessageModel). */
    void setMessageModel(MessageModel *model);

    /** Get the ChatMessageView for a given row (nullptr if off-screen). */
    ChatMessageView* widgetForMessage(int row) const;

    /** Get the row index for a given widget pointer (-1 if not found). */
    int rowForWidget(ChatMessageView *widget) const;

    /** Scroll to the bottom (deferred via event loop). */
    void scrollToBottomDeferred();

    /** Scroll to the bottom immediately. */
    void scrollToBottomImmediate();

    /**
     * Mark a row's widget as needing re-creation (e.g. after streaming
     * content changes the cached data in the widget).
     */
    void scheduleWidgetUpdate(int row);

    /** Create / destroy widgets based on current visible range (public for streaming sync). */
    void updateVisibleWidgets();

    /** Suppress scroll-to-top signal during message prepend operations. */
    void setPrependInProgress(bool v) { m_prependInProgress = v; }

signals:
    /** Emitted after a new ChatMessageView is created for a visible row.
     *  Allows the session page to restore streaming state, etc. */
    void widgetCreated(int row, ChatMessageView *widget);

    /** Emitted when the user scrolls near the top (for loading older messages). */
    void scrollToTopRequested();

protected:
    void scrollContentsBy(int dx, int dy) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onRowsInserted(const QModelIndex &parent, int first, int last);
    void onModelReset();
    void onMessageContentChanged(int row);

private:

    /** Build a ChatMessageView for the given row from model data. */
    ChatMessageView* buildWidget(int row);

    /** Remove all cached widget references. */
    void clearWidgetCache();

    /** Determine the visible row range with buffer. */
    void visibleRange(int &firstRow, int &lastRow) const;

    MessageModel *m_model = nullptr;
    QTimer *m_updateTimer = nullptr;
    bool m_updateScheduled = false;
    bool m_autoScrollEnabled = true;
    bool m_prependInProgress = false;

    /** Row -> widget cache for O(1) lookup during streaming. */
    QHash<int, ChatMessageView*> m_widgetCache;
};

} // namespace ady

#endif // MESSAGE_LIST_VIEW_H
