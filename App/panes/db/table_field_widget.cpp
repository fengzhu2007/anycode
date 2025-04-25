#include "table_field_widget.h"

#include "table_field_model.h"
#include "dbms_pane.h"
#include "db_driver.h"
#include "sqlite/field_form.h"
#include "components/message_dialog.h"
#include <w_toast.h>
#include <QToolBar>
#include <QAction>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>
#include <QTimer>
#include <QSqlError>


namespace Ui {
class TableFieldWidget{
public :
    QToolBar* toolBar;
    QSplitter* splitter;
    QTreeView* treeView;
    ady::FieldFormImpl* form;
    QAction* actionSave;
    QAction* actionAdd;
    QAction* actionRemove;
    QAction* actionUp;
    QAction* actionDown;



    void setupUi(QWidget* parent){
        auto layout = new QVBoxLayout();
        layout->setMargin(0);
        parent->setLayout(layout);


        this->toolBar = new QToolBar(parent);
        toolBar->setMovable(false);
        toolBar->setIconSize(QSize(16, 16));

        this->actionAdd = new QAction(QIcon(":/Resource/icons/Add_16x.svg"),QObject::tr("Add"),parent);
        this->actionSave = new QAction(QIcon(":/Resource/icons/Save_16x.svg"),QObject::tr("Save"),parent);
        this->actionRemove = new QAction(QIcon(":/Resource/icons/Cancel_16x.svg"),QObject::tr("Remove"),parent);


        this->toolBar->addAction(this->actionSave);
        this->toolBar->addSeparator();
        this->toolBar->addAction(this->actionAdd);
        this->toolBar->addAction(this->actionRemove);


        this->splitter = new QSplitter(Qt::Vertical,parent);

        layout->addWidget(this->toolBar);
        layout->addWidget(this->splitter,1);

        this->treeView = new QTreeView(this->splitter);
        this->treeView->setRootIsDecorated(false);
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
    QMap<QString,TableField> modifications;
    QList<TableField> fields;//ori fields
};

TableFieldWidget::TableFieldWidget(long long id,const QString& table,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TableFieldWidget)
{
    ui->setupUi(this);

    ui->form = new ady::sqlite::FieldForm(id,ui->splitter);
    ui->splitter->addWidget(ui->form);







    d = new TableFieldWidgetPrivate;
    d->id = id;
    d->name = table;

    d->model = new ady::TableFieldModel(ui->treeView);
    ui->treeView->setModel(d->model);
    ui->treeView->setAlternatingRowColors(true);
    //ui->treeView->setShowGrid(true);
    connect(ui->treeView,&QAbstractItemView::clicked,this,&TableFieldWidget::onFieldActivated);
    connect(ui->form,&sqlite::FieldForm::change,this,&TableFieldWidget::onFieldChanged);

    connect(ui->actionAdd,&QAction::triggered,this,&TableFieldWidget::onActionTriggered);
    connect(ui->actionSave,&QAction::triggered,this,&TableFieldWidget::onActionTriggered);
    connect(ui->actionRemove,&QAction::triggered,this,&TableFieldWidget::onActionTriggered);

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
            d->fields = driver->tableFields(d->name);
            d->model->setDatasource(d->fields);
        }
    }
}

void TableFieldWidget::onFieldActivated(const QModelIndex& index){
    auto field = d->model->at(index.row());
    ui->form->init(field);
}

void TableFieldWidget::onActionTriggered(){
    auto sender = this->sender();
    if(sender==ui->actionAdd){
        d->model->appendItem({});
        auto index = d->model->index(d->model->rowCount() - 1,0);
        ui->treeView->selectionModel()->select(index,QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        ui->treeView->scrollTo(index);
        this->onFieldActivated(index);
    }else if(sender==ui->actionSave){
        auto driver = DBMSPane::getInstance()->connector(d->id);
        auto ret = driver->updateTableFields(d->name,d->fields,d->model->fields());
        if(ret==false){
            //show error message
            auto error = driver->lastError();
            if(error.type()!=QSqlError::NoError){
                wToast::showText(tr("SQL Error:%1").arg(error.databaseText()));
            }
        }else{
            this->initData();
            wToast::showText(tr("Save successfully"));
        }
    }else if(sender==ui->actionRemove){
        //auto list = ui->treeView->selectionModel()->selectedIndexes();
        auto index = ui->treeView->selectionModel()->currentIndex();
        if(index.isValid()){
            if(MessageDialog::confirm(this,tr("Delete Feild Confirm"),tr("Are you sure you want to delete the current field?"))==QMessageBox::Yes){
                d->model->removeItem(index.row());
                this->notifyFieldsChanged();
            }
        }
    }
}

void TableFieldWidget::onFieldChanged(const QString& name,TableField* field){
    //d->modifications.insert(name,*field);
    d->model->updateItem(*field);
    this->notifyFieldsChanged();

}

void TableFieldWidget::notifyFieldsChanged(){
    auto list = d->model->fields();
    QStringList array;
    for(auto one:list){
        array.append(one.name);
    }
    emit fieldsChanged(array);
}

}

