#include "session_page_widget.h"
#include "ui_session_page_widget.h"
#include "chat_service.h"
#include "core/theme.h"
#include <QKeyEvent>
#include <QTextCursor>
#include <QDebug>

namespace ady {

SessionPageWidget::SessionPageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SessionPageWidget)
{
    ui->setupUi(this);

    ui->messageContainer->setSizeConstraint(QLayout::SetMinAndMaxSize);
    ui->messageContainer->setAlignment(Qt::AlignTop);
    ui->messageScrollContents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // Enter 
    ui->messageInput->installEventFilter(this);
}

SessionPageWidget::~SessionPageWidget()
{
    delete ui;
}

QScrollArea* SessionPageWidget::messageScrollArea() const { return ui->messageScrollArea; }
QWidget* SessionPageWidget::messageScrollContents() const { return ui->messageScrollContents; }
QVBoxLayout* SessionPageWidget::messageContainer() const { return ui->messageContainer; }
QComboBox* SessionPageWidget::modelCombo() const { return ui->modelCombo; }
QTextEdit* SessionPageWidget::messageInput() const { return ui->messageInput; }
QToolButton* SessionPageWidget::sendBtn() const { return ui->sendBtn; }

void SessionPageWidget::appendInputText(const QString &text)
{
    ui->messageInput->moveCursor(QTextCursor::End);
    ui->messageInput->insertPlainText(text);
    ui->messageInput->setFocus();
}

void SessionPageWidget::setModels(const QList<OpenCodeModel> &models, const QString &selectedData)
{
    ui->modelCombo->blockSignals(true);
    ui->modelCombo->clear();

    for(const auto &m : models){
        QString displayName = m.name.isEmpty() ? m.modelID : m.name;
        QString data = m.providerID + "/" + m.modelID;
        ui->modelCombo->addItem(displayName, data);
    }
    qDebug()<<"setModels"<<selectedData;

    if(!selectedData.isEmpty()){
        int idx = ui->modelCombo->findData(selectedData);
        if(idx >= 0){
            ui->modelCombo->setCurrentIndex(idx);
        }
    }
    ui->modelCombo->blockSignals(false);
}

bool SessionPageWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->messageInput && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (keyEvent->modifiers() & Qt::AltModifier) {
                ui->messageInput->insertPlainText("\n");
                return true;
            } else {
                emit enterPressed();
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

}
