#include "session_list_popup.h"
#include "components/listview/listview.h"
#include "core/theme.h"
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QFocusEvent>

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

    setStyleSheet("SessionListItemWidget { background: palette(base); }");

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
    m_closeBtn->setIcon(QIcon(":/Resource/icons/Cancel_16x.svg"));
    m_closeBtn->setIconSize(QSize(12, 12));
    m_closeBtn->setFixedSize(18, 18);
    m_closeBtn->setStyleSheet("QToolButton { border: none; background: transparent; }"
                              "QToolButton:hover { background: rgba(128,128,128,40); border-radius: 3px; }");
    m_closeBtn->setCursor(Qt::ArrowCursor);
    layout->addWidget(m_closeBtn);

    connect(m_closeBtn, &QToolButton::clicked, [this]() {
        emit closeClicked(m_sessionId);
    });
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

void SessionListItemWidget::mousePressEvent(QMouseEvent *event)
{
    handleClick();
    ListViewItem::mousePressEvent(event);
}

void SessionListItemWidget::handleClick()
{
    emit clicked(m_sessionId);
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
    setAttribute(Qt::WA_TranslucentBackground, false);

    QString borderColor = Theme::getInstance()->borderColor().name(QColor::HexRgb);
    setStyleSheet(
        "SessionListPopup { border: 1px solid " + borderColor + "; background: palette(window); }"
    );

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet("QScrollArea { border: none; background: palette(base); }");

    m_container = new QWidget();
    m_container->setStyleSheet("background: palette(base);");
    m_itemsLayout = new QVBoxLayout(m_container);
    m_itemsLayout->setContentsMargins(0, 0, 0, 0);
    m_itemsLayout->setSpacing(1);

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
    Q_UNUSED(currentSessionId);

    qDeleteAll(m_items);
    m_items.clear();

    for (const auto &s : sessions) {
        QString title = s.title.isEmpty() ? tr("New Chat") : s.title;
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

    if (sessions.isEmpty()) {
        auto *label = new QLabel(tr("No Sessions"), m_container);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("QLabel { color: palette(mid); padding: 20px; background: palette(base); }");
        m_itemsLayout->addWidget(label);
    }
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
    move(globalPos);
    show();
    int contentHeight = m_container->sizeHint().height() + 2; // +2 for border
    int maxH = maximumHeight();
    setFixedHeight(qMin(contentHeight, maxH));
}

void SessionListPopup::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    hide();
}

}
