#include "session_config_dialog.h"
#include "ui_session_config_dialog.h"
#include <QFileDialog>

namespace ady {

SessionConfigDialog::SessionConfigDialog(QWidget *parent)
    : wDialog(parent)
    , ui(new Ui::SessionConfigDialog)
{
    ui->setupUi(this);

    connect(ui->browseBtn, &QPushButton::clicked, this, [this]() {
        QString currentDir = ui->dirCombo->currentText().trimmed();
        QString dir = QFileDialog::getExistingDirectory(
            this, tr("Select Working Directory"), currentDir);
        if (!dir.isEmpty())
            ui->dirCombo->setEditText(dir);
    });

    connect(ui->saveBtn, &QPushButton::clicked, this, [this]() {
        if (ui->titleEdit->text().trimmed().isEmpty()) {
            ui->titleEdit->setFocus();
            return;
        }
        accept();
    });

    connect(ui->cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    resetupUi();
    setWindowTitle(tr("Session Settings"));
    setFixedWidth(420);
    adjustSize();
}

SessionConfigDialog::~SessionConfigDialog()
{
    delete ui;
}

void SessionConfigDialog::setSessionTitle(const QString &title)
{
    ui->titleEdit->setText(title);
}

void SessionConfigDialog::setPreference(const QString &preference)
{
    ui->prefEdit->setPlainText(preference);
}

void SessionConfigDialog::setDirectory(const QString &dir)
{
    ui->dirCombo->setEditText(dir);
}

void SessionConfigDialog::setDirectoryList(const QStringList &paths)
{
    ui->dirCombo->clear();
    ui->dirCombo->addItems(paths);
}

QString SessionConfigDialog::sessionTitle() const
{
    return ui->titleEdit->text().trimmed();
}

QString SessionConfigDialog::preference() const
{
    return ui->prefEdit->toPlainText().trimmed();
}

QString SessionConfigDialog::directory() const
{
    return ui->dirCombo->currentText().trimmed();
}

}
