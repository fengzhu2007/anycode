#include "file_diff_item_widget.h"
#include "ui_file_diff_item_widget.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event_data.h"
#include "core/event_bus/publisher.h"
#include <QFileInfo>
#include <QMouseEvent>

namespace ady {

FileDiffItemWidget::FileDiffItemWidget(const FileDiffInfo &info, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FileDiffItemWidget)
    , m_filePath(info.file)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::NoFocus);

    setStyleSheet("FileDiffItemWidget{border-radius: 4px;}");

    // Status badge
    ui->badgeLabel->setText(statusIcon(info.status));
    ui->badgeLabel->setStyleSheet(
        QString("font-weight: bold; font-size: 11px; color: %1;").arg(statusColor(info.status))
    );

    // File name
    QString fileName = QFileInfo(info.file).fileName();
    QString dirPath = QFileInfo(info.file).path();
    ui->nameLabel->setText(fileName);
    ui->nameLabel->setCursor(Qt::PointingHandCursor);
    ui->nameLabel->installEventFilter(this);

    // Stats line: dir path + additions/deletions
    QString statsText;
    if (!dirPath.isEmpty() && dirPath != ".") {
        statsText = dirPath + "  ";
    }
    if (info.additions > 0) {
        statsText += QString("<span style='color:#4caf50;'>+%1</span> ").arg(info.additions);
    }
    if (info.deletions > 0) {
        statsText += QString("<span style='color:#f44336;'>-%1</span>").arg(info.deletions);
    }
    ui->statsLabel->setText(statsText);

    // Reject button style
    ui->rejectBtn->setFocusPolicy(Qt::NoFocus);
    ui->rejectBtn->setStyleSheet(
        "QToolButton{background: transparent; color: rgba(255,100,100,180); border: 1px solid rgba(255,100,100,80);"
        "border-radius: 3px; font-size: 13px; padding: 0; min-width: 0; min-height: 0;}"
        "QToolButton:hover{background: rgba(255,50,50,60); color: rgba(255,100,100,255);}"
    );
    ui->rejectBtn->setToolTip(tr("Reject this file"));

    // Accept button style
    ui->acceptBtn->setFocusPolicy(Qt::NoFocus);
    ui->acceptBtn->setStyleSheet(
        "QToolButton{background: transparent; color: rgba(100,255,100,180); border: 1px solid rgba(100,255,100,80);"
        "border-radius: 3px; font-size: 13px; padding: 0; min-width: 0; min-height: 0;}"
        "QToolButton:hover{background: rgba(50,255,50,60); color: rgba(100,255,100,255);}"
    );
    ui->acceptBtn->setToolTip(tr("Accept this file"));

    ui->actionStatusLabel->hide();

    // Connect signals
    connect(ui->rejectBtn, &QToolButton::clicked, this, [this]() {
        m_actionState = Rejected;
        ui->rejectBtn->hide();
        ui->acceptBtn->hide();
        ui->actionStatusLabel->setText(tr("已拒绝"));
        ui->actionStatusLabel->setStyleSheet("color: rgba(255,255,255,150); font-size: 11px;");
        ui->actionStatusLabel->show();
        emit rejected(m_filePath);
    });
    connect(ui->acceptBtn, &QToolButton::clicked, this, [this]() {
        m_actionState = Accepted;
        ui->rejectBtn->hide();
        ui->acceptBtn->hide();
        ui->actionStatusLabel->setText(tr("已接受"));
        ui->actionStatusLabel->setStyleSheet("color: rgba(255,255,255,150); font-size: 11px;");
        ui->actionStatusLabel->show();
        emit accepted(m_filePath);
    });
}

FileDiffItemWidget::~FileDiffItemWidget()
{
    delete ui;
}

QString FileDiffItemWidget::statusIcon(const QString &status)
{
    if (status == "added") return "A";
    if (status == "deleted") return "D";
    return "M";
}

QString FileDiffItemWidget::statusColor(const QString &status)
{
    if (status == "added") return "#4caf50";
    if (status == "deleted") return "#f44336";
    return "#ff9800";
}

void FileDiffItemWidget::setActionState(ActionState state)
{
    if (m_actionState == state) return;
    m_actionState = state;
    if (state == None) {
        ui->rejectBtn->show();
        ui->acceptBtn->show();
        ui->actionStatusLabel->hide();
    } else {
        ui->rejectBtn->hide();
        ui->acceptBtn->hide();
        ui->actionStatusLabel->setText(state == Accepted ? tr("已接受") : tr("已拒绝"));
        ui->actionStatusLabel->setStyleSheet("color: rgba(255,255,255,150); font-size: 11px;");
        ui->actionStatusLabel->show();
    }
}

bool FileDiffItemWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->nameLabel && event->type() == QEvent::MouseButtonPress) {
        OpenEditorData data{m_filePath, 0, 0, false};
        Publisher::getInstance()->post(Type::M_OPEN_EDITOR, &data);
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

} // namespace ady
