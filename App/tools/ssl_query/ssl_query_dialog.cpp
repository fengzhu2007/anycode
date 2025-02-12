#include "ssl_query_dialog.h"
#include "ui_ssl_query_dialog.h"
#include "core/schedule/schedule.h"
#include "ssl_query_task.h"
#include "query_result_model.h"
#include "ssl_querier.h"
#include "components/message_dialog.h"
#include "storage/common_storage.h"
#include <w_toast.h>
#include <QLineEdit>
#include <QTimeZone>
#include <QDebug>


namespace ady{

static QString SSL_QUERIER_KEY = "ssl_querier_domains";

SSLQueryDialog* SSLQueryDialog::instance = nullptr;
class SSLQueryDialogPrivate{
public:
    QLineEdit* editor;
    QueryResultModel* model;

};

SSLQueryDialog::SSLQueryDialog(QWidget* parent):wDialog(parent),ui(new Ui::SSLQueryDialog) {
    d = new SSLQueryDialogPrivate;
    ui->setupUi(this);
    this->resetupUi();
    d->editor = new QLineEdit(ui->toolBar);
    //d->editor->setMaximumWidth(200);
    d->editor->setMaximumHeight(22);
    ui->toolBar->insertWidget(ui->actionSearch,d->editor);
    d->model = new QueryResultModel(ui->treeView);
    ui->treeView->setModel(d->model);
    ui->treeView->setColumnWidth(QueryResultModel::Domain,200);
    ui->treeView->setColumnWidth(QueryResultModel::StartDate,120);
    ui->treeView->setColumnWidth(QueryResultModel::ExpireDate,120);


    connect(ui->actionAdd,&QAction::triggered,this,&SSLQueryDialog::onActionTrigger);
    connect(ui->actionSearch,&QAction::triggered,this,&SSLQueryDialog::onActionTrigger);
    connect(ui->actionDelete,&QAction::triggered,this,&SSLQueryDialog::onActionTrigger);
    connect(ui->actionRefresh_All,&QAction::triggered,this,&SSLQueryDialog::onActionTrigger);
    this->onQuery();
}

SSLQueryDialog::~SSLQueryDialog(){
    delete ui;
    delete d;
    instance = nullptr;
}

SSLQueryDialog* SSLQueryDialog::getInstance(){
    return instance;
}

SSLQueryDialog* SSLQueryDialog::open(QWidget* parent){
    if(instance==nullptr){
        instance = new SSLQueryDialog(parent);
    }
    instance->show();
    return instance;
}


static bool isValidDomain(const QString &domain) {
    QRegularExpression regex(
        R"(^(?:[a-z0-9](?:[a-z0-9-]{0,61}[a-z0-9])?\.)+[a-z0-9][a-z0-9-]{0,61}[a-z0-9]$)",
        QRegularExpression::CaseInsensitiveOption
        );

    return regex.match(domain).hasMatch();
}

void SSLQueryDialog::onActionTrigger(){
    auto sender = this->sender();
    if(sender==ui->actionAdd){
        auto text = d->editor->text().trimmed().toLower();
        if(text.isEmpty()==false && isValidDomain(text)){
            this->queryDomains({text});

            auto domains = d->model->allDomains();
            CommonStorage().replace(SSL_QUERIER_KEY,domains.join(","));//save
        }else{
            wToast::showText(tr("Invalid domain"));
        }
    }else if(sender==ui->actionSearch){
        auto text = d->editor->text().trimmed().toLower();
        auto model = ui->treeView->selectionModel();
        model->clear();
        if(!text.isEmpty()){
            //find
            auto list = d->model->search(text);
            for(auto one:list){
                model->select(one, QItemSelectionModel::Select | QItemSelectionModel::Rows);
            }
        }
    }else if(sender==ui->actionDelete){
        if(MessageDialog::confirm(this,tr("Are you sure you want to delete the selected domain(s)?"))==QMessageBox::Yes){
            //auto list = ui->treeView->selectionMode()
            auto list = ui->treeView->selectionModel()->selectedRows();
            QStringList array;
            for(auto one:list){
                auto item = d->model->itemAt(one.row());
                array << item.domain;
            }
            for(auto one:array){
                d->model->removeItem(one);
            }
            auto domains = d->model->allDomains();

            CommonStorage().replace(SSL_QUERIER_KEY,domains.join(","));//save
        }
    }else if(sender==ui->actionRefresh_All){
        d->model->clear();
        this->onQuery();
    }
}

void SSLQueryDialog::onQuery(){
    //CommonStorage().replace(SSL_QUERIER_KEY,domains.join(","));//get
    auto r = CommonStorage().one(SSL_QUERIER_KEY);
    if(!r.value.isEmpty()){
        //QStringList sites = {"www.guzheng.cn","www.hqgq.com","www.guoqinwang.com","m.guzheng.cn","www.baidu.com"};
        QStringList sites = r.value.split(",");
        this->queryDomains(sites);
    }
}

void SSLQueryDialog::onOneReady(const QString& domain,const QJsonObject& data){
    QString format = "MMM d hh:mm:ss yyyy 'GMT'";
    auto startDate = parseDateTime(data.find("StartDate")->toString());
    auto expireDate = parseDateTime(data.find("ExpireDate")->toString());
    QueryResult result{domain,startDate,expireDate};
    d->model->updateItem(result);
}

void SSLQueryDialog::onOneError(const QString& domain,const QString& errorMsg){
    QueryResult result{domain,{},{},errorMsg};
    d->model->updateItem(result);
}

void SSLQueryDialog::onFinish(){
    ui->widget->stop();
    ui->actionAdd->setEnabled(true);
    ui->actionSearch->setEnabled(true);
    ui->actionDelete->setEnabled(true);
    ui->actionRefresh_All->setEnabled(true);
    d->model->sortAll();
}

QDateTime SSLQueryDialog::parseDateTime(const QString& dateTimeString){
    QStringList parts = dateTimeString.simplified().split(' ');
    if (parts.size() == 5) {
        QString monthStr = parts[0]; // "Jul"
        int day = parts[1].toInt();  // 8
        QString timeStr = parts[2];  // "01:41:02"
        int year = parts[3].toInt(); // 2024
        QString timeZone = parts[4]; // "GMT"
        QMap<QString, int> monthMap = {
            {"Jan", 1}, {"Feb", 2}, {"Mar", 3}, {"Apr", 4},
            {"May", 5}, {"Jun", 6}, {"Jul", 7}, {"Aug", 8},
            {"Sep", 9}, {"Oct", 10}, {"Nov", 11}, {"Dec", 12}
        };
        int month = monthMap.value(monthStr, -1);
        if (month != -1) {
            QStringList timeParts = timeStr.split(':');
            if (timeParts.size() == 3) {
                int hour = timeParts[0].toInt();
                int minute = timeParts[1].toInt();
                int second = timeParts[2].toInt();
                QDate date(year, month, day);
                QTime time(hour, minute, second);
                QDateTime dateTime(date, time,Qt::UTC);
                return dateTime;
            }
        }
    }
    return {};
}

void SSLQueryDialog::queryDomains(const QStringList& sites){
    auto instance = Schedule::getInstance();
    auto task = new SSLQueryTask(sites);
    auto querier = task->querier();
    connect(querier,&SSLQuerier::oneReady,this,&SSLQueryDialog::onOneReady);
    connect(querier,&SSLQuerier::oneError,this,&SSLQueryDialog::onOneError);
    connect(querier,&SSLQuerier::finish,this,&SSLQueryDialog::onFinish);
    instance->addTask(task);
    qDebug()<<"list"<<sites;
    ui->widget->start();
    QList<QueryResult> list;
    for(auto one:sites){
        QueryResult item {one,{},{},{}};
        list<<item;
    }
    if(d->model->rowCount()==0){
        d->model->setDataSource(list);
    }else{
        for(auto one:list){
            d->model->appendItem(one);
        }
    }
    ui->actionAdd->setEnabled(false);
    ui->actionSearch->setEnabled(false);
    ui->actionDelete->setEnabled(false);
    ui->actionRefresh_All->setEnabled(false);
}

}
