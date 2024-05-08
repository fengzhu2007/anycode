#include "StatusBarView.h"
#include "ui_StatusBarView.h"
namespace ady{
    StatusBarView::StatusBarView(QWidget* parent)
        :QWidget(parent),
        ui(new Ui::StatusBarViewUi)
    {
        ui->setupUi(this);
        m_isOnline = true;
    }

    bool StatusBarView::networkStatus()
    {
        return m_isOnline;
    }

    void StatusBarView::setNetworkStatus(bool isOnline)
    {

        if(m_isOnline!=isOnline){
            if(isOnline){
                ui->label->setPixmap(QPixmap(QString::fromUtf8(":/img/Resource/net_status_ok.png")));
            }else{
                ui->label->setPixmap(QPixmap(QString::fromUtf8(":/img/Resource/net_status_error.png")));
            }
        }

        m_isOnline = isOnline;
    }

    void StatusBarView::showMessage(const QString& message)
    {
        ui->message->setText(message);
    }

    QString StatusBarView::currentMessage()
    {
        return ui->message->text();
    }


}
