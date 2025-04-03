#include "table_list_widget.h"
#include "ui_table_list_widget.h"

#include "table_data_model.h"
#include "dbms_pane.h"
#include "db_driver.h"

namespace ady{
class TableListWidgetPrivate{
public:
    long long id;
    QString name;
    TableDataModel* model;
};

TableListWidget::TableListWidget(long long id,const QString& table,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TableListWidget)
{
    ui->setupUi(this);
    d = new TableListWidgetPrivate;
    d->id = id;
    d->name = table;

    d->model = new TableDataModel(ui->tableView);
    ui->tableView->setModel(d->model);
    //ui->tableView->verticalHeader()->setMinimumWidth(30);
    auto verticalHeader = ui->tableView->verticalHeader();
    verticalHeader->setMinimumWidth(16);
    verticalHeader->setDefaultSectionSize(24);


    ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);


    this->initData();
}

TableListWidget::~TableListWidget()
{
    delete ui;
    delete d;
}

void TableListWidget::initData(){
    auto instance = DBMSPane::getInstance();
    if(instance){
        auto driver = instance->connector(d->id);
        if(driver){
            auto result = driver->queryData(d->name);
            auto fields = std::get<0>(result);
            QList<QList<QVariant>> data = std::get<1>(result);
            long long total = std::get<2>(result);
            d->model->setDatasource(fields,data);
        }
    }
}


}
