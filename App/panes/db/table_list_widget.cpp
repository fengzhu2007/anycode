#include "table_list_widget.h"
#include "ui_table_list_widget.h"

#include "table_data_model.h"
#include "dbms_pane.h"
#include "db_driver.h"
#include "components/message_dialog.h"

namespace ady{
class TableListWidgetPrivate{
public:
    long long id;
    QString name;
    TableDataModel* model;
    QList<QSqlField> fields;
    QList<QList<QVariant>> data;
    long long total;
    int page = 1;
    int num = 100;
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

    connect(ui->actionAdd,&QAction::triggered,this,&TableListWidget::onActionTriggered);
    connect(ui->actionSave,&QAction::triggered,this,&TableListWidget::onActionTriggered);
    connect(ui->actionSave_All,&QAction::triggered,this,&TableListWidget::onActionTriggered);
    connect(ui->actionDelete,&QAction::triggered,this,&TableListWidget::onActionTriggered);
    connect(ui->actionPrevious,&QAction::triggered,this,&TableListWidget::onActionTriggered);
    connect(ui->actionNext,&QAction::triggered,this,&TableListWidget::onActionTriggered);
    connect(ui->actionRefresh,&QAction::triggered,this,&TableListWidget::onActionTriggered);

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
            int offset = (d->page - 1) * d->num;
            auto result = driver->queryData(d->name,{},{},{},offset,d->num);
            d->fields = std::get<0>(result);
            d->data = std::get<1>(result);
            d->total = std::get<2>(result);
            d->model->setDatasource(d->fields,d->data);

            ui->actionPrevious->setEnabled(d->page>1);
            ui->actionNext->setEnabled(d->data.size()>=d->num);
        }
    }
}

void TableListWidget::setTableName(const QString& tableName){
    d->name = tableName;
}

void TableListWidget::onActionTriggered(){
    auto sender = this->sender();
    if(sender==ui->actionSave){
        auto driver = DBMSPane::getInstance()->connector(d->id);
        if(driver){
            auto data = d->model->changedData();
            if(data.size()>0){
                auto iter = data.begin();
                while(iter!=data.end()){
                    auto row = iter.key();
                    QList<QVariant> oData;
                    bool ret = false;
                    if(row >= d->data.length()){
                        auto nData = iter.value();
                        ret = driver->insert(d->name,d->fields,nData);
                        if(ret){
                            d->data.append(nData);
                            d->model->updateItem(row,nData);
                        }
                    }else{
                        oData = d->data.at(row);
                        ret = driver->update(d->name,d->fields,oData,iter.value());
                        if(ret){
                            d->data[row] = iter.value();
                            d->model->clearChanged(row);
                        }else{
                            //driver->err
                            MessageDialog::error(this,driver->errorText());
                            return ;
                        }
                    }
                    iter++;
                }
            }
        }
    }else if(sender==ui->actionAdd){
        d->model->appendRow();
    }else if(sender==ui->actionDelete){
        QModelIndex index = ui->tableView->selectionModel()->currentIndex();
        if(index.isValid()){
            int row = index.row();
            if(row >= d->data.size()){
                d->model->removeRow(row);
                return ;
            }
            auto item = d->data.at(row);
            auto driver = DBMSPane::getInstance()->connector(d->id);
            if(driver){
                if(MessageDialog::confirm(this,tr("Are you want to delete current record?"))==QMessageBox::Yes){
                    auto ret = driver->del(d->name,d->fields,item);
                    if(ret){
                        d->model->removeRow(row);
                    }else{
                        MessageDialog::error(this,driver->errorText());
                    }

                }



            }
        }
    }else if(sender==ui->actionPrevious){
        if(d->page>1){
            d->page -= 1;
        }
        this->initData();
    }else if(sender==ui->actionNext){
        d->page +=1 ;
        this->initData();
    }else if(sender==ui->actionRefresh){
        this->initData();
    }
}

}
