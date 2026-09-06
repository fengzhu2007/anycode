#include "session_list_popup_v2.h"
#include "session_list_popup.h" // reuse SessionListItemWidget
#include "core/theme.h"

#include <QPainter>
#include <QMouseEvent>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QScrollBar>

namespace ady {

// ============================================================================
// SessionListModelV2
// ============================================================================

SessionListModelV2::SessionListModelV2(QObject *parent)
    : QAbstractListModel(parent)
{
}

int SessionListModelV2::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_sessions.size();
}

QVariant SessionListModelV2::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_sessions.size())
        return {};

    const auto &s = m_sessions.at(index.row());
    if (role == Qt::DisplayRole) {
        return s.title.isEmpty() ? tr("New Chat") : s.title;
    }
    return {};
}

void SessionListModelV2::setSessions(const QList<OpenCodeSession> &sessions)
{
    beginResetModel();
    m_sessions = sessions;
    endResetModel();
}

void SessionListModelV2::updateTitle(const QString &sessionId, const QString &title)
{
    for (int i = 0; i < m_sessions.size(); ++i) {
        if (m_sessions[i].id == sessionId) {
            m_sessions[i].title = title;
            emit dataChanged(index(i), index(i), {Qt::DisplayRole});
            break;
        }
    }
}

void SessionListModelV2::updateStatus(const QString &sessionId, bool busy)
{
    Q_UNUSED(sessionId);
    Q_UNUSED(busy);
    // Status is managed by the popup's updateSessionStatus() via index widgets
}

OpenCodeSession SessionListModelV2::sessionAt(int row) const
{
    if (row >= 0 && row < m_sessions.size())
        return m_sessions.at(row);
    return {};
}

// ============================================================================
// SessionListDelegateV2
// ============================================================================

SessionListDelegateV2::SessionListDelegateV2(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QSize SessionListDelegateV2::sizeHint(const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(200, 32);
}

void SessionListDelegateV2::paint(QPainter *painter,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(index);
    // Rendering is done by index widgets, not the delegate.
}

// ============================================================================
// SessionListPopupV2
// ============================================================================

SessionListPopupV2::SessionListPopupV2(QWidget *parent)
    : QWidget(parent)
    , m_animScheduled(false)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    auto theme = Theme::getInstance();
    const QString bg = theme->backgroundColor().name(QColor::HexRgb);
    setStyleSheet(QString("SessionListPopupV2 { background: %1; border-radius: 8px; }").arg(bg));

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(10, 10, 10, 10);
    outerLayout->setSpacing(0);

    m_listView = new QListView(this);
    m_listView->setFrameShape(QFrame::NoFrame);
    m_listView->setSelectionMode(QAbstractItemView::NoSelection);
    m_listView->setFocusPolicy(Qt::NoFocus);
    m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_listView->setSpacing(8);
    m_listView->setUniformItemSizes(false);
    m_listView->setStyleSheet("QListView { border: none; background: transparent; }");

    m_model = new SessionListModelV2(this);
    m_delegate = new SessionListDelegateV2(this);
    m_listView->setModel(m_model);
    m_listView->setItemDelegate(m_delegate);

    outerLayout->addWidget(m_listView);

    // Empty label overlay
    m_emptyLabel = new QLabel(tr("No Sessions"), this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("QLabel { color: palette(mid); background: transparent; }");
    m_emptyLabel->hide();

    setMinimumWidth(220);
    setMaximumHeight(400);
}

SessionListPopupV2::~SessionListPopupV2()
{
}

void SessionListPopupV2::refresh(const QList<OpenCodeSession> &sessions,
                                  const QString &currentSessionId)
{
    Q_UNUSED(currentSessionId);
    m_pendingSessions = sessions;
}

void SessionListPopupV2::updateSessionTitle(const QString &sessionId, const QString &title)
{
    m_model->updateTitle(sessionId, title);
    for (auto *w : m_indexWidgets) {
        if (w->sessionId() == sessionId) {
            w->setTitle(title);
            break;
        }
    }
}

void SessionListPopupV2::updateSessionStatus(const QString &sessionId, bool busy)
{
    for (auto *w : m_indexWidgets) {
        if (w->sessionId() == sessionId) {
            w->setStatusIcon(busy);
            break;
        }
    }
}

void SessionListPopupV2::showAt(const QPoint &globalPos)
{
    // Apply pending data
    m_model->setSessions(m_pendingSessions);



    int count = m_model->rowCount();

    // Empty state
    m_emptyLabel->setVisible(count == 0);
    if (count == 0) {
        setFixedHeight(80);
        m_emptyLabel->setGeometry(rect());
    }

    m_popupPos = globalPos;

    // Disable scrollbar BEFORE show
    m_listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Hide viewport during setup to prevent flash
   // m_listView->viewport()->hide();

    // Set full height immediately (no dynamic growth)
    // Layout margins (10+10) provide top/bottom padding automatically
    const int itemHeight = 32;
    const int spacing = 8;
    int contentHeight = count * itemHeight + qMax(0, count - 1) * spacing;
    // Add layout margins to get total widget height
    int totalHeight = contentHeight + 20; // 10 top + 10 bottom margin
    setGeometry(m_popupPos.x(), m_popupPos.y(), width(), qMin(totalHeight, maximumHeight()));
    //qDebug()<<"showAt:"<<this->geometry();


    //setMaximumHeight(0);
    if (count == 0) return;
    m_listView->viewport()->update();
    m_listView->viewport()->repaint();
    // Create index widgets while viewport is hidden
    ensureIndexWidgets();



    show();
    startCascadeAnimation();
}

void SessionListPopupV2::ensureIndexWidgets()
{
    static QString suffix = "111";

    for (int i = 0; i < m_model->rowCount(); ++i) {
        m_listView->setIndexWidget(m_model->index(i), nullptr);
    }

    for (auto *w : m_indexWidgets) {
        w->deleteLater();
    }
    m_indexWidgets.clear();

    m_listView->viewport()->update();
    QCoreApplication::processEvents();

    int count = m_model->rowCount();
    for (int i = 0; i < count; ++i) {
        auto s = m_model->sessionAt(i);
        QString title = s.title.isEmpty() ? tr("New Chat") : s.title;
        auto *w = new SessionListItemWidget(title+"--"+suffix, s.id, m_listView->viewport());
        // Start fully transparent; cascade animation will fade in
        auto *effect = new QGraphicsOpacityEffect(w);
        effect->setOpacity(0.0);
        w->setGraphicsEffect(effect);
        connect(w, &SessionListItemWidget::clicked, [this](const QString &sessionId) {
            emit sessionClicked(sessionId);
            startCloseAnimation();
        });
        connect(w, &SessionListItemWidget::closeClicked, [this](const QString &sessionId) {
            emit sessionCloseClicked(sessionId);
        });
        m_listView->setIndexWidget(m_model->index(i), w);
        m_indexWidgets.append(w);

    }

    suffix = "222";
}

void SessionListPopupV2::startCascadeAnimation()
{
   // if (m_animScheduled) return;
   // m_animScheduled = true;

    const int staggerDelay = 40;
    for (int i = 0; i < m_indexWidgets.size(); ++i) {
        auto *item = m_indexWidgets[i];
        QTimer::singleShot(i * staggerDelay, this, [item]() {
            if (!item) return;

            // Reuse the existing opacity effect (set to 0 in ensureIndexWidgets)
            auto *effect = qobject_cast<QGraphicsOpacityEffect*>(item->graphicsEffect());
            if (!effect) {
                effect = new QGraphicsOpacityEffect(item);
                item->setGraphicsEffect(effect);
            }

            auto *fadeAnim = new QPropertyAnimation(effect, "opacity", item);
            fadeAnim->setDuration(150);
            fadeAnim->setStartValue(0.0);
            fadeAnim->setEndValue(1.0);
            fadeAnim->setEasingCurve(QEasingCurve::OutCubic);
            fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
        });
    }
}

void SessionListPopupV2::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
}

void SessionListPopupV2::startCloseAnimation()
{
    // Collect items that are currently opaque (opacity > 0)
    QList<SessionListItemWidget*> opaqueItems;
    for (auto *w : m_indexWidgets) {
        if (!w) continue;
        w->setVisible(false);
    }
    hide();
}

void SessionListPopupV2::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    startCloseAnimation();
    m_listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

}



} // namespace ady
