#include "table_field_widget.h"

#include "table_field_model.h"
#include "dbms_pane.h"
#include "db_driver.h"
#include <QToolBar>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>
#include <QTimer>


namespace Ui {
class TableFieldWidget{
public :
    QToolBar* toolBar;
    QSplitter* splitter;
    QTreeView* treeView;





    void setupUi(QWidget* parent){
        auto layout = new QVBoxLayout();
        parent->setLayout(layout);

        this->toolBar = new QToolBar(parent);
        this->splitter = new QSplitter(parent);

        layout->addWidget(this->toolBar);
        layout->addWidget(this->splitter,1);

        this->treeView = new QTreeView(this->splitter);
        this->splitter->addWidget(this->treeView);



    }
};
}



namespace  ady {
class TableFieldWidgetPrivate{
public:
    long long id;
    QString name;
    ady::TableFieldModel* model;
};

TableFieldWidget::TableFieldWidget(long long id,const QString& table,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TableFieldWidget)
{
    ui->setupUi(this);
    d = new TableFieldWidgetPrivate;
    d->id = id;
    d->name = table;

    d->model = new ady::TableFieldModel(ui->treeView);
    ui->treeView->setModel(d->model);


    this->initData();
}

TableFieldWidget::~TableFieldWidget()
{
    delete ui;
    delete d;
}

void TableFieldWidget::initData(){
    auto instance = DBMSPane::getInstance();
    if(instance){
        auto driver = instance->connector(d->id);
        if(driver){
            auto fields = driver->tableFields(d->name);
            d->model->setDatasource(fields);


        }
    }
}


}

