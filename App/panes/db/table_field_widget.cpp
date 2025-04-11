#include "table_field_widget.h"

#include "table_field_model.h"
#include "dbms_pane.h"
#include "db_driver.h"
#include "sqlite/field_form.h"
#include "components/tree_item_delegate.h"
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
    ady::FieldFormImpl* form;



    void setupUi(QWidget* parent){
        auto layout = new QVBoxLayout();
        layout->setMargin(0);
        parent->setLayout(layout);


        this->toolBar = new QToolBar(parent);
        this->splitter = new QSplitter(Qt::Vertical,parent);

        layout->addWidget(this->toolBar);
        layout->addWidget(this->splitter,1);

        this->treeView = new QTreeView(this->splitter);
        this->treeView->setRootIsDecorated(false);
        this->splitter->addWidget(this->treeView);

        this->form = new ady::sqlite::FieldForm(this->splitter);
        this->splitter->addWidget(this->form);



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
    connect(ui->treeView,&QAbstractItemView::activated,this,&TableFieldWidget::onFieldActivated);
    //ui->treeView->setItemDelegate(new TreeItemDelegate(ui->treeView));



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

void TableFieldWidget::onFieldActivated(const QModelIndex& index){
    auto field = d->model->at(index.row());
    ui->form->init(field);
}

}

