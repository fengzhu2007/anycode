#include "status_bar_view.h"
#include "ui_status_bar_view.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event.h"
#include "panes/file_transfer/file_transfer_model.h"
#include "common/utils.h"
#include <w_badge.h>
#include <QStyleOption>
#include <QPainter>
#include <QStyle>
#include <QResizeEvent>
namespace ady{
StatusBarView* StatusBarView::instance = nullptr;

    StatusBarView::StatusBarView(QWidget* parent)
        :QWidget(parent),
        ui(new Ui::StatusBarView)
    {
        this->reg();
        this->regMessageIds({Type::M_MESSAGE,Type::M_READY,Type::M_NEW_NOTIFICATION});

        this->setStyleSheet(QString::fromUtf8(".QLabel{color:white;}"));
        ui->setupUi(this);
        m_isOnline = true;
        this->setReady();
        this->setRate({-1,-1});

        auto label = new QLabel(ui->notification);
        label->setPixmap(QPixmap(QString::fromUtf8(":/Resource/icons/NotificationAlert_16x.svg")));
        ui->notification->setCentralWidget(label);
        //ui->notification->setCount(99);

        connect(ui->notification,&wBadge::clicked,this,&StatusBarView::onToggleNotification);

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
                ui->network->setPixmap(QPixmap(QString::fromUtf8(":/Resource/icons/NetworkStatus_16x.svg")));
                ui->network->setToolTip(tr("Network:OK"));
            }else{
                ui->network->setPixmap(QPixmap(QString::fromUtf8(":/Resource/icons/NetworkStatusWarning_16x.svg")));
                ui->network->setToolTip(tr("Network:Error"));
            }
            //update file transfer status
            auto instance = FileTransferModel::getInstance();
            if(instance){
                instance->setOnline(isOnline);
            }
        }
        m_isOnline = isOnline;
    }

    void StatusBarView::onToggleNotification(){
        qDebug()<<"onToggleNotification";
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
        }else if(id==Type::M_READY){
            this->setReady();
            return true;
        }else if(id==Type::M_NEW_NOTIFICATION){
            auto count = static_cast<int*>(e->data());
            //qDebug()<<"new notification"<<(*count);
            ui->notification->setCount(ui->notification->count() + (*count));
        }
        return false;
    }

    void StatusBarView::setReady(){
        ui->message->setText(tr("Ready"));
    }

    void StatusBarView::setRate(const QPair<long long,long long>& rate){
        if(rate.first==-1 || rate.second==-1){
            ui->rate->hide();
            ui->rateImg->hide();
        }else{
            ui->rateImg->show();
            ui->rate->show();
            const QString text = tr("Upload:%1/s, Download:%2/s").arg(Utils::readableFilesize(rate.first,1)).arg(Utils::readableFilesize(rate.second,1));
            ui->rate->setText(text);
        }
    }


    StatusBarView* StatusBarView::getInstance(){
        return instance;
    }

    StatusBarView* StatusBarView::make(QWidget* parent){
        if(instance==nullptr){
            instance = new StatusBarView(parent);
            return instance;
        }else{
            return nullptr;
        }
    }


}
