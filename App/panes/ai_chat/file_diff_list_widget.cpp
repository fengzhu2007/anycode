#include "file_diff_list_widget.h"
#include "file_diff_item_widget.h"
#include "ui_file_diff_list_widget.h"
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

    // Disable item selection but keep hover tracking
    ui->listWidget->setFocusPolicy(Qt::NoFocus);
    ui->listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

FileDiffListWidget::~FileDiffListWidget()
{
    delete ui;
}

void FileDiffListWidget::appendDiffs(const QList<FileDiffInfo> &diffs)
{
    if (diffs.isEmpty()) return;

    ui->emptyLabel->hide();
    ui->listWidget->show();

    for (const auto &info : diffs) {
        auto *itemWidget = new FileDiffItemWidget(info);
        connect(itemWidget, &FileDiffItemWidget::rejected, this, &FileDiffListWidget::fileRejected);
        connect(itemWidget, &FileDiffItemWidget::accepted, this, &FileDiffListWidget::fileAccepted);

        auto *item = new QListWidgetItem;
        item->setSizeHint(QSize(0, 36));
        item->setFlags(Qt::ItemIsEnabled);
        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item, itemWidget);
    }

    int total = ui->listWidget->count();
    int maxItems = qMin(total, 8);
    ui->listWidget->setMaximumHeight(qMax(maxItems * 36, 40));
}

void FileDiffListWidget::clear()
{
    m_diffs.clear();
    ui->listWidget->clear();
    ui->listWidget->hide();
    ui->emptyLabel->show();
}

void FileDiffListWidget::markAllAccepted()
{
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        auto *w = qobject_cast<FileDiffItemWidget*>(ui->listWidget->itemWidget(ui->listWidget->item(i)));
        if (w && w->actionState() == FileDiffItemWidget::None)
            w->setActionState(FileDiffItemWidget::Accepted);
    }
}

void FileDiffListWidget::markAllRejected()
{
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        auto *w = qobject_cast<FileDiffItemWidget*>(ui->listWidget->itemWidget(ui->listWidget->item(i)));
        if (w && w->actionState() == FileDiffItemWidget::None)
            w->setActionState(FileDiffItemWidget::Rejected);
    }
}

bool FileDiffListWidget::hasPendingItems() const
{
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        auto *w = qobject_cast<FileDiffItemWidget*>(ui->listWidget->itemWidget(ui->listWidget->item(i)));
        if (w && w->actionState() == FileDiffItemWidget::None)
            return true;
    }
    return false;
}

}
