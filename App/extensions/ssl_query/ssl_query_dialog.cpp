#include "ssl_query_dialog.h"
#include "ui_ssl_query_dialog.h"
#include "core/schedule/schedule.h"
#include "ssl_query_task.h"
#include "query_result_model.h"
#include "ssl_querier.h"

#include <QLineEdit>
#include <QTimeZone>
#include <QDebug>


namespace ady{

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

void SSLQueryDialog::onQuery(){
    auto instance = Schedule::getInstance();
    QStringList sites = {"www.guzheng.cn","www.hqgq.com","www.guoqinwang.com","m.guzheng.cn","www.baidu.com"};
    auto task = new SSLQueryTask(sites);
    /*connect(task,&SSLQueryTask::oneReady,this,&SSLQueryDialog::onOneReady);
    connect(task,&SSLQueryTask::oneError,this,&SSLQueryDialog::onOneError);
    connect(task,&SSLQueryTask::finish,this,&SSLQueryDialog::onFinish);*/
    auto querier = task->querier();
    connect(querier,&SSLQuerier::oneReady,this,&SSLQueryDialog::onOneReady);
    connect(querier,&SSLQuerier::oneError,this,&SSLQueryDialog::onOneError);
    connect(querier,&SSLQuerier::finish,this,&SSLQueryDialog::onFinish);
    instance->addTask(task);
    ui->widget->start();
    QList<QueryResult> list;
    for(auto one:sites){
        QueryResult item {one};
        list<<item;
    }
    d->model->setDataSource(list);
    ui->actionAdd->setEnabled(false);
    ui->actionSearch->setEnabled(false);
}

void SSLQueryDialog::onOneReady(const QString& domain,const QJsonObject& data){
    QString format = "MMM d hh:mm:ss yyyy 'GMT'";
    auto startDate = parseDateTime(data.find("StartDate")->toString());
    auto expireDate = parseDateTime(data.find("ExpireDate")->toString());

    QueryResult result{domain,startDate,expireDate};
    d->model->updateItem(result);
}

void SSLQueryDialog::onOneError(const QString& domain,const QString& errorMsg){
    qDebug()<<"error"<<domain<<errorMsg;
}

void SSLQueryDialog::onFinish(){
    ui->widget->stop();
    ui->actionAdd->setEnabled(true);
    ui->actionSearch->setEnabled(true);
}


QDateTime SSLQueryDialog::parseDateTime(const QString& dateTimeString){
    QStringList parts = dateTimeString.simplified().split(' ');
    qDebug()<<"dateTimeString"<<dateTimeString;
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

}
