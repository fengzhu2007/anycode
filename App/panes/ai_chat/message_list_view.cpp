/**
 * @file message_list_view.cpp
 * @brief Virtualized QListView implementation for chat messages.
 *
 * Key design:
 *  - Uses QListView::setIndexWidget() to place ChatMessageWidget only for
 *    visible rows (+ a buffer of 3 rows above/below).
 *  - When rows scroll out of the visible+buffer range, their widgets are
 *    destroyed, freeing memory and reducing layout cost.
 *  - A coalescing timer prevents excessive widget churn during fast scrolling.
 *  - The widget cache (QHash<int, ChatMessageWidget*>) gives O(1) lookup
 *    for streaming updates without iterating the entire list.
 */
#include "message_list_view.h"
#include "message_model.h"
#include "chat_message_widget.h"
#include "message_delegate.h"

#include <QScrollBar>
#include <QTimer>
#include <QResizeEvent>
#include <QDebug>

namespace ady {

static const int BUFFER_ROWS = 3;

MessageListView::MessageListView(QWidget *parent)
    : QListView(parent)
{
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setSelectionMode(QAbstractItemView::NoSelection);
    setFocusPolicy(Qt::NoFocus);
    setFrameShape(QFrame::NoFrame);
    setSpacing(10);
    setUniformItemSizes(false);
    setItemDelegate(new MessageDelegate(this));

    // Coalescing timer: debounces rapid scroll/resize events
    m_updateTimer = new QTimer(this);
    m_updateTimer->setSingleShot(true);
    m_updateTimer->setInterval(0);
    connect(m_updateTimer, &QTimer::timeout, this, [this]() {
        m_updateScheduled = false;
        updateVisibleWidgets();
    });

    // Auto-scroll when scrollbar reaches bottom
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
        m_autoScrollEnabled = (value >= verticalScrollBar()->maximum() - 10);
    });
}

void MessageListView::setMessageModel(MessageModel *model)
{
    m_model = model;
    setModel(model);

    connect(model, &QAbstractItemModel::rowsInserted,
            this, &MessageListView::onRowsInserted);
    connect(model, &QAbstractItemModel::modelReset,
            this, &MessageListView::onModelReset);
}

// ---- public API ----

ChatMessageWidget* MessageListView::widgetForMessage(int row) const
{
    return m_widgetCache.value(row, nullptr);
}

int MessageListView::rowForWidget(ChatMessageWidget *widget) const
{
    for (auto it = m_widgetCache.cbegin(); it != m_widgetCache.cend(); ++it) {
        if (it.value() == widget)
            return it.key();
    }
    return -1;
}

void MessageListView::scrollToBottomDeferred()
{
    QTimer::singleShot(0, this, [this]() {
        scrollToBottomImmediate();
    });
}

void MessageListView::scrollToBottomImmediate()
{
    QScrollBar *sb = verticalScrollBar();
    if (sb)
        sb->setValue(sb->maximum());
}

void MessageListView::scheduleWidgetUpdate(int row)
{
    if (m_model && row >= 0 && row < m_model->rowCount()) {
        m_model->notifyDataChanged(row);
    }
}

// ---- overrides ----

void MessageListView::scrollContentsBy(int dx, int dy)
{
    QListView::scrollContentsBy(dx, dy);

    if (!m_updateScheduled) {
        m_updateScheduled = true;
        m_updateTimer->start();
    }
}

void MessageListView::resizeEvent(QResizeEvent *event)
{
    QListView::resizeEvent(event);

    if (!m_updateScheduled) {
        m_updateScheduled = true;
        m_updateTimer->start();
    }
}

// ---- model signal handlers ----

void MessageListView::onRowsInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);

    bool lastRow = (last == m_model->rowCount() - 1);
    if (lastRow && m_autoScrollEnabled) {
        scrollToBottomDeferred();
    }

    // Ensure newly added rows get widgets if visible
    if (!m_updateScheduled) {
        m_updateScheduled = true;
        m_updateTimer->start();
    }
}

void MessageListView::onModelReset()
{
    clearWidgetCache();
    if (!m_updateScheduled) {
        m_updateScheduled = true;
        m_updateTimer->start();
    }
}

void MessageListView::onMessageContentChanged(int row)
{
    if (m_model && row >= 0 && row < m_model->rowCount()) {
        m_model->notifyDataChanged(row);
    }
}

// ---- private implementation ----

void MessageListView::visibleRange(int &firstRow, int &lastRow) const
{
    if (!m_model || m_model->rowCount() == 0) {
        firstRow = 0;
        lastRow = -1;
        return;
    }

    int total = m_model->rowCount();
    QRect vr = viewport()->rect();

    // Find first visible row
    firstRow = indexAt(vr.topLeft()).row();
    if (firstRow < 0) {
        // Viewport might be larger than content - show all
        firstRow = 0;
        lastRow = total - 1;
        return;
    }

    // Find last visible row
    lastRow = indexAt(vr.bottomLeft()).row();
    if (lastRow < 0) {
        lastRow = total - 1;
    }

    // Add buffer
    firstRow = qMax(0, firstRow - BUFFER_ROWS);
    lastRow = qMin(total - 1, lastRow + BUFFER_ROWS);
}

void MessageListView::updateVisibleWidgets()
{
    if (!m_model) return;

    int firstVisible, lastVisible;
    visibleRange(firstVisible, lastVisible);

    // 1. Destroy widgets for rows outside visible range
    QList<int> toRemove;
    for (auto it = m_widgetCache.begin(); it != m_widgetCache.end(); ++it) {
        int row = it.key();
        if (row < firstVisible || row > lastVisible) {
            setIndexWidget(m_model->index(row), nullptr);
            toRemove.append(row);
        }
    }
    for (int row : toRemove) {
        m_widgetCache.remove(row);
    }

    // 2. Create widgets for newly visible rows
    for (int row = firstVisible; row <= lastVisible; ++row) {
        if (m_widgetCache.contains(row))
            continue;

        ChatMessageWidget *w = buildWidget(row);
        if (!w) continue;

        setIndexWidget(m_model->index(row), w);
        m_widgetCache[row] = w;

        // Connect signals
        connect(w, &ChatMessageWidget::contentUpdated, this, [this, row]() {
            onMessageContentChanged(row);
        });

        // Notify that a new widget was created (for streaming state restore, etc.)
        emit widgetCreated(row, w);
    }
}

ChatMessageWidget* MessageListView::buildWidget(int row)
{
    if (!m_model || row < 0 || row >= m_model->rowCount())
        return nullptr;

    MessageData msg = m_model->messageAt(row);
    auto *w = new ChatMessageWidget(msg.type, msg.content, viewport());
    return w;
}

void MessageListView::clearWidgetCache()
{
    // setIndexWidget(nullptr) on all cached rows to destroy widgets
    if (m_model) {
        for (auto it = m_widgetCache.begin(); it != m_widgetCache.end(); ++it) {
            setIndexWidget(m_model->index(it.key()), nullptr);
        }
    }
    m_widgetCache.clear();
}

} // namespace ady
