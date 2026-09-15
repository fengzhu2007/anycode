#include "file_diff_list_widget.h"
#include "ui_file_diff_list_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QFileInfo>
#include <QListWidgetItem>

namespace ady {

FileDiffListWidget::FileDiffListWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FileDiffListWidget)
{
    ui->setupUi(this);

    // Hide list initially, show empty label
    ui->listWidget->hide();
    ui->emptyLabel->show();

    // Connect button signals
    connect(ui->rejectBtn, &QPushButton::clicked, this, &FileDiffListWidget::rejectAll);
    connect(ui->acceptBtn, &QPushButton::clicked, this, &FileDiffListWidget::acceptAll);
}

FileDiffListWidget::~FileDiffListWidget()
{
    delete ui;
}

void FileDiffListWidget::setDiffs(const QList<FileDiffInfo> &diffs)
{
    m_diffs = diffs;
    ui->listWidget->clear();

    bool hasItems = !diffs.isEmpty();
    ui->listWidget->setVisible(hasItems);
    ui->emptyLabel->setVisible(!hasItems);
    ui->acceptBtn->setEnabled(hasItems);
    ui->rejectBtn->setEnabled(hasItems);

    for (int i = 0; i < diffs.size(); ++i) {
        createFileItem(i);
    }

    // Adjust list height based on item count (max 8 items visible)
    int itemHeight = 36;
    int maxItems = qMin(diffs.size(), 8);
    int maxHeight = maxItems * itemHeight;
    //ui->listWidget->setFixedHeight(qMax(maxHeight, 40));
    ui->listWidget->setMaximumHeight(qMax(maxHeight, 40));
}

void FileDiffListWidget::clear()
{
    m_diffs.clear();
    ui->listWidget->clear();
    ui->listWidget->setVisible(false);
    ui->emptyLabel->setVisible(true);
    ui->acceptBtn->setEnabled(false);
    ui->rejectBtn->setEnabled(false);
}

void FileDiffListWidget::createFileItem(int index)
{
    const FileDiffInfo &info = m_diffs.at(index);

    auto *item = new QListWidgetItem;
    item->setSizeHint(QSize(0, 36));
    item->setFlags(Qt::ItemIsEnabled);
    ui->listWidget->addItem(item);

    // Container widget
    auto *container = new QWidget;
    auto *layout = new QHBoxLayout(container);
    layout->setContentsMargins(8, 2, 8, 2);
    layout->setSpacing(6);

    // Status badge
    auto *badge = new QLabel(statusIcon(info.status));
    badge->setFixedWidth(18);
    badge->setAlignment(Qt::AlignCenter);
    badge->setStyleSheet(
        QString("font-weight: bold; font-size: 11px; color: %1;").arg(statusColor(info.status))
    );
    layout->addWidget(badge);

    // File name + stats
    auto *fileInfo = new QWidget;
    auto *fileLayout = new QVBoxLayout(fileInfo);
    fileLayout->setContentsMargins(0, 0, 0, 0);
    fileLayout->setSpacing(0);

    // Extract just the filename for display
    QString fileName = QFileInfo(info.file).fileName();
    QString dirPath = QFileInfo(info.file).path();
    auto *nameLabel = new QLabel(fileName);
    nameLabel->setStyleSheet("font-size: 12px; color: rgba(255,255,255,220);");
    nameLabel->setToolTip(info.file);
    fileLayout->addWidget(nameLabel);

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
    auto *statsLabel = new QLabel(statsText);
    statsLabel->setTextFormat(Qt::RichText);
    statsLabel->setStyleSheet("font-size: 12px; color: rgba(255,255,255,100);");
    fileLayout->addWidget(statsLabel);

    layout->addWidget(fileInfo, 1);

    // Per-file reject button
    auto *rejectBtn = new QToolButton;
    rejectBtn->setText(QString::fromUtf8("\xc3\x97"));  // × character
    rejectBtn->setFixedSize(22, 22);
    rejectBtn->setCursor(Qt::PointingHandCursor);
    rejectBtn->setToolTip(tr("Reject this file"));
    rejectBtn->setStyleSheet(
        "QToolButton{background: transparent; color: rgba(255,100,100,180); border: 1px solid rgba(255,100,100,80);"
        "border-radius: 3px; font-size: 13px; padding: 0; min-width: 0; min-height: 0;}"
        "QToolButton:hover{background: rgba(255,50,50,60); color: rgba(255,100,100,255);}"
    );
    QString rejectFile = info.file;
    connect(rejectBtn, &QToolButton::clicked, this, [this, rejectFile]() {
        emit fileRejected(rejectFile);
    });
    layout->addWidget(rejectBtn);

    // Per-file accept button
    auto *acceptBtn = new QToolButton;
    acceptBtn->setText(QString::fromUtf8("\xe2\x9c\x93"));  // ✓ character
    acceptBtn->setFixedSize(22, 22);
    acceptBtn->setCursor(Qt::PointingHandCursor);
    acceptBtn->setToolTip(tr("Accept this file"));
    acceptBtn->setStyleSheet(
        "QToolButton{background: transparent; color: rgba(100,255,100,180); border: 1px solid rgba(100,255,100,80);"
        "border-radius: 3px; font-size: 13px; padding: 0; min-width: 0; min-height: 0;}"
        "QToolButton:hover{background: rgba(50,255,50,60); color: rgba(100,255,100,255);}"
    );
    QString acceptFile = info.file;
    connect(acceptBtn, &QToolButton::clicked, this, [this, acceptFile]() {
        emit fileAccepted(acceptFile);
    });
    layout->addWidget(acceptBtn);

    ui->listWidget->setItemWidget(item, container);
}

QString FileDiffListWidget::statusIcon(const QString &status)
{
    if (status == "added") return "A";
    if (status == "deleted") return "D";
    return "M";
}

QString FileDiffListWidget::statusColor(const QString &status)
{
    if (status == "added") return "#4caf50";
    if (status == "deleted") return "#f44336";
    return "#ff9800";
}

}
