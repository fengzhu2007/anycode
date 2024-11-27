#include "status_bar_view.h"
#include "ui_status_bar_view.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event.h"
namespace ady{
    StatusBarView::StatusBarView(QWidget* parent)
        :QWidget(parent),
        ui(new Ui::StatusBarView)
    {
        this->reg();
        this->regMessageId({Type::M_MESSAGE});
        this->setStyleSheet(QString::fromUtf8(".QLabel{color:white;}"));
        ui->setupUi(this);
        m_isOnline = true;
        ui->message->setText(tr("Ready"));
    }

    StatusBarView::~StatusBarView(){
        this->unReg();
        delete ui;
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

    bool StatusBarView::onReceive(Event* e){
        auto id = e->id();
        if(id==Type::M_MESSAGE){
            auto text = static_cast<QString*>(e->data());
            showMessage(*text);
            return true;
        }
        return false;
    }


}
