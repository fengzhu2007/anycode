#include "session_list_popup.h"
#include "components/listview/listview.h"
#include "core/theme.h"
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QPainter>
#include <QStyleOption>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QWindow>
#include <QEvent>

namespace ady {

// ============================================================================
// SessionListItemWidget
// ============================================================================

SessionListItemWidget::SessionListItemWidget(const QString &title, const QString &sessionId, QWidget *parent)
    : ListViewItem(parent)
    , m_sessionId(sessionId)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 4, 6, 4);
    layout->setSpacing(4);

    auto theme = Theme::getInstance();
    const QString bg = theme->secondaryBackgroundColor().name(QColor::HexRgb);
    setStyleSheet(
        QString("ady--SessionListItemWidget { background-color: %1; border-radius: 6px; }").arg(bg)
    );

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_titleLabel->setStyleSheet("QLabel { background: transparent; color: palette(text); border: none; }");
    layout->addWidget(m_titleLabel, 1);

    m_statusIcon = new QLabel(this);
    m_statusIcon->setFixedSize(16, 16);
    m_statusIcon->setPixmap(QIcon(":/Resource/icons/NetworkStatus_16x.svg").pixmap(16, 16));
    m_statusIcon->setToolTip(tr("Idle"));
    m_statusIcon->setStyleSheet("QLabel { background: transparent; border: none; }");
    layout->addWidget(m_statusIcon);

    m_closeBtn = new QToolButton(this);
    m_closeBtn->setIcon(QIcon(":/Resource/icons/close.svg"));
    m_closeBtn->setIconSize(QSize(12, 12));
    m_closeBtn->setFixedSize(18, 18);
    m_closeBtn->setStyleSheet("QToolButton { border: none; background: transparent; }"
                              "QToolButton:hover { background: rgba(128,128,128,40); border-radius: 3px; }");
    m_closeBtn->setCursor(Qt::ArrowCursor);
    m_closeBtn->installEventFilter(this);
    layout->addWidget(m_closeBtn);

    setFixedHeight(32);

    connect(m_closeBtn, &QToolButton::clicked, [this]() {
        emit closeClicked(m_sessionId);
    });
}

bool SessionListItemWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_closeBtn) {
        if (event->type() == QEvent::Enter) {
            m_closeBtn->setIcon(QIcon(":/Resource/icons/close_hover.svg"));
        } else if (event->type() == QEvent::Leave) {
            m_closeBtn->setIcon(QIcon(":/Resource/icons/close.svg"));
        }
    }
    return ListViewItem::eventFilter(obj, event);
}

void SessionListItemWidget::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
}

void SessionListItemWidget::setStatusIcon(bool busy)
{
    if (busy) {
        m_statusIcon->setPixmap(QIcon(":/Resource/icons/Hourglass_16x.svg").pixmap(16, 16));
        m_statusIcon->setToolTip(tr("Requesting..."));
    } else {
        m_statusIcon->setPixmap(QIcon(":/Resource/icons/NetworkStatus_16x.svg").pixmap(16, 16));
        m_statusIcon->setToolTip(tr("Idle"));
    }
}

void SessionListItemWidget::setCurrent(bool current)
{
    auto theme = Theme::getInstance();
    QColor bg = theme->secondaryBackgroundColor();
    if (current) {
        bg = bg.lighter(130);
    }
    const QString bgStr = bg.name(QColor::HexRgb);
    setStyleSheet(
        QString("ady--SessionListItemWidget { background-color: %1; border-radius: 6px; }").arg(bgStr)
    );
}

void SessionListItemWidget::mousePressEvent(QMouseEvent *event)
{
    handleClick();
    ListViewItem::mousePressEvent(event);
}

void SessionListItemWidget::handleClick()
{
    emit clicked(m_sessionId);
}

void SessionListItemWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

// ============================================================================
// SessionListModel
// ============================================================================

SessionListModel::SessionListModel(ListView *parent)
    : ListViewModel(parent)
{
}

int SessionListModel::count()
{
    return m_items.size();
}

ListViewItem* SessionListModel::item(int i)
{
    if (i >= 0 && i < m_items.size()) {
        return m_items.at(i);
    }
    return nullptr;
}

ListViewItem* SessionListModel::takeAt(int i)
{
    if (i >= 0 && i < m_items.size()) {
        return m_items.takeAt(i);
    }
    return nullptr;
}

QWidget* SessionListModel::emptyWidget()
{
    if (!m_emptyWidget) {
        m_emptyWidget = new QLabel(listView()->widget());
        m_emptyWidget->setAlignment(Qt::AlignCenter);
        m_emptyWidget->setText(QString::fromUtf8("<br/><br/>%1").arg(tr("No Sessions")));
    }
    m_emptyWidget->show();
    return m_emptyWidget;
}

void SessionListModel::refresh(const QList<OpenCodeSession> &sessions)
{
    qDeleteAll(m_items);
    m_items.clear();

    for (const auto &s : sessions) {
        QString title = s.title.isEmpty() ? tr("New Chat") : s.title;
        auto *item = new SessionListItemWidget(title, s.id, listView()->widget());
        m_items.append(item);
        if(m_itemCreatedCallback){
            m_itemCreatedCallback(item);
        }
    }

    dataChanged();
}

void SessionListModel::setItemCreatedCallback(std::function<void(SessionListItemWidget*)> callback)
{
    m_itemCreatedCallback = callback;
}

void SessionListModel::updateSessionTitle(const QString &sessionId, const QString &title)
{
    for (auto *item : m_items) {
        if (item->sessionId() == sessionId) {
            item->setTitle(title);
            break;
        }
    }
}

void SessionListModel::updateSessionStatus(const QString &sessionId, bool busy)
{
    for (auto *item : m_items) {
        if (item->sessionId() == sessionId) {
            item->setStatusIcon(busy);
            break;
        }
    }
}

// ============================================================================
// SessionListPopup
// ============================================================================

SessionListPopup::SessionListPopup(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    //setAttribute(Qt::WA_NoSystemBackground, true);



    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    m_container = new QWidget();
    m_container->setStyleSheet("background: transparent;");
    m_itemsLayout = new QVBoxLayout(m_container);
    m_itemsLayout->setContentsMargins(10, 10, 10, 10);
    m_itemsLayout->setSpacing(8);

    m_scrollArea->setWidget(m_container);
    outerLayout->addWidget(m_scrollArea);

    setMinimumWidth(220);
    setMaximumHeight(400);
}

SessionListPopup::~SessionListPopup()
{
}

void SessionListPopup::refresh(const QList<OpenCodeSession> &sessions, const QString &currentSessionId)
{
    m_currentSessionId = currentSessionId;

    // Hide existing items immediately so they don't flash if the
    // popup is currently visible when refresh() is called.
    for (auto *item : m_items) {
        item->hide();
    }

    // Only store data; widgets are created in showAt() to avoid
    // items flashing on screen before the cascade animation.
    qDeleteAll(m_items);
    m_items.clear();
    m_pendingSessions = sessions;

    // Rebuild visible items to reflect updated data immediately
    while (m_itemsLayout->count() > 0) {
        auto *item = m_itemsLayout->takeAt(0)->widget();
        if (item) delete item;
    }
    for (int i = 0; i < sessions.size(); ++i) {
        const auto &s = sessions[i];
        QString title = s.title.isEmpty() ? tr("New Chat") : s.title;
        auto *item = new SessionListItemWidget(title, s.id, m_container);
        item->setCurrent(s.id == m_currentSessionId);
        connect(item, &SessionListItemWidget::clicked, [this](const QString &sessionId) {
            emit sessionClicked(sessionId);
            hide();
        });
        connect(item, &SessionListItemWidget::closeClicked, [this](const QString &sessionId) {
            emit sessionCloseClicked(sessionId);
        });
        m_itemsLayout->addWidget(item);
        m_items.append(item);
    }

    // Update popup height for new item count
    const int itemHeight = 32;
    const int spacing = 8;
    const int margins = 20;
    int count = m_items.size();
    int contentHeight = (count == 0) ? 80 : (count * itemHeight + qMax(0, count - 1) * spacing + margins);
    setFixedHeight(qMin(contentHeight, maximumHeight()));
}

void SessionListPopup::updateSessionTitle(const QString &sessionId, const QString &title)
{
    for (auto *item : m_items) {
        if (item->sessionId() == sessionId) {
            item->setTitle(title);
            break;
        }
    }
}

void SessionListPopup::updateSessionStatus(const QString &sessionId, bool busy)
{
    for (auto *item : m_items) {
        if (item->sessionId() == sessionId) {
            item->setStatusIcon(busy);
            break;
        }
    }
}

void SessionListPopup::showAt(const QPoint &globalPos)
{
    const auto &sessions = m_pendingSessions;
    setUpdatesEnabled(false);
    m_container->hide();

    while (m_items.size() > sessions.size()) {
        auto *item = m_items.takeLast();
        m_itemsLayout->removeWidget(item);
        delete item;
    }
    for (int i = 0; i < sessions.size(); ++i) {
        const auto &s = sessions[i];
        QString title = s.title.isEmpty() ? tr("New Chat") : s.title;
        if (i < m_items.size()) {
            m_items[i]->setTitle(title);
            m_items[i]->setSessionId(s.id);
        } else {
            auto *item = new SessionListItemWidget(title, s.id, m_container);
            connect(item, &SessionListItemWidget::clicked, [this](const QString &sessionId) {
                emit sessionClicked(sessionId);
                hide();
            });
            connect(item, &SessionListItemWidget::closeClicked, [this](const QString &sessionId) {
                emit sessionCloseClicked(sessionId);
            });
            m_itemsLayout->addWidget(item);
            m_items.append(item);
        }
        m_items[i]->setGraphicsEffect(nullptr);
        m_items[i]->setCurrent(sessions[i].id == m_currentSessionId);
        m_items[i]->hide();
    }

    QLabel *emptyLabel = nullptr;
    for (int i = 0; i < m_itemsLayout->count(); ++i) {
        auto *w = m_itemsLayout->itemAt(i)->widget();
        if (w && !m_items.contains(static_cast<SessionListItemWidget*>(w))) {
            emptyLabel = qobject_cast<QLabel*>(w);
            break;
        }
    }
    if (sessions.isEmpty() && !emptyLabel) {
        emptyLabel = new QLabel(tr("No Sessions"), m_container);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("QLabel { color: palette(mid); padding: 20px; background: transparent; }");
        emptyLabel->hide();
        m_itemsLayout->addWidget(emptyLabel);
    }
    if (emptyLabel) emptyLabel->setVisible(sessions.isEmpty());

    const int itemHeight = 32;
    const int spacing = 8;
    const int margins = 20;
    int count = m_items.size();
    int contentHeight = (count == 0) ? 80 : (count * itemHeight + qMax(0, count - 1) * spacing + margins);

    move(globalPos);
    setFixedHeight(qMin(contentHeight, maximumHeight()));

    m_container->show();
    setUpdatesEnabled(true); //

    if (windowHandle()) {
        windowHandle()->destroy();
        windowHandle()->create();
    }

    show();

    const int staggerDelay = 40;
    for (int i = 0; i < m_items.size(); ++i) {
        auto *item = m_items[i];
        QTimer::singleShot(i * staggerDelay, this, [item]() {
            if (!item) return;

            auto *effect = new QGraphicsOpacityEffect(item);
            item->setGraphicsEffect(effect);

            auto *fadeAnim = new QPropertyAnimation(effect, "opacity", item);
            fadeAnim->setDuration(150);
            fadeAnim->setStartValue(0.0);
            fadeAnim->setEndValue(1.0);
            fadeAnim->setEasingCurve(QEasingCurve::OutCubic);

            connect(fadeAnim, &QPropertyAnimation::finished, item, [item]() {
                item->setGraphicsEffect(nullptr);
            });

            fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
            item->show();
        });
    }
}

void SessionListPopup::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    hide();
}

void SessionListPopup::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);

    // Hide all items when popup closes so they don't flash
    // on the next show() before the cascade animation starts.
    for (auto *item : m_items) {
        item->hide();
    }
    QPalette pal = palette();
    pal.setBrush(QPalette::Window, Qt::transparent);
    setPalette(pal);
    repaint();
    QCoreApplication::processEvents();

}

void SessionListPopup::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(event->rect(), QColor(0, 0, 0,120));

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    QWidget::paintEvent(event);
}

}
