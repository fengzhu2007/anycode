#include "table_index_widget.h"
#include "sqlite/index_form.h"
#include "table_index_model.h"
#include "components/message_dialog.h"
#include "dbms_pane.h"
#include "db_driver.h"
#include <w_toast.h>
#include <QToolBar>
#include <QAction>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>
#include <QTimer>
#include <QSqlError>

namespace Ui {
class TableIndexWidget{
public :
    QToolBar* toolBar;
    QSplitter* splitter;
    QTreeView* treeView;
    ady::IndexFormImpl* form;
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



namespace ady{
class TableIndexWidgetPrivate{
public:
    long long id;
    QString name;
    ady::TableIndexModel* model;
    QMap<QString,TableField> modifications;
    QList<TableIndex> indexes;//ori fields
};

TableIndexWidget::TableIndexWidget(long long id,const QString& table,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TableIndexWidget)
{
    ui->setupUi(this);

    ui->form = new ady::sqlite::IndexForm(id,ui->splitter);
    ui->splitter->addWidget(ui->form);

    d = new TableIndexWidgetPrivate;
    d->id = id;
    d->name = table;

    d->model = new ady::TableIndexModel(ui->treeView);
    ui->treeView->setModel(d->model);
    ui->treeView->setAlternatingRowColors(true);
    //ui->treeView->setShowGrid(true);
    connect(ui->treeView,&QAbstractItemView::clicked,this,&TableIndexWidget::onFieldActivated);
    connect(ui->form,&sqlite::IndexForm::change,this,&TableIndexWidget::onIndexChanged);

    connect(ui->actionAdd,&QAction::triggered,this,&TableIndexWidget::onActionTriggered);
    connect(ui->actionSave,&QAction::triggered,this,&TableIndexWidget::onActionTriggered);
    connect(ui->actionRemove,&QAction::triggered,this,&TableIndexWidget::onActionTriggered);

    this->initData();


}

TableIndexWidget::~TableIndexWidget()
{
    delete d;
    delete ui;
}


void TableIndexWidget::initData(){
    auto instance = DBMSPane::getInstance();
    if(instance){
        auto driver = instance->connector(d->id);
        if(driver){
            d->indexes = driver->tableIndexes(d->name);
            d->model->setDatasource(d->indexes);
        }
    }
}

void TableIndexWidget::setTableName(const QString& tableName){
    d->name = tableName;
}

void TableIndexWidget::onFieldActivated(const QModelIndex& index){
    auto field = d->model->at(index.row());
    ui->form->init(field);
}

void TableIndexWidget::onActionTriggered(){
    auto sender = this->sender();
    if(sender==ui->actionAdd){
        d->model->appendItem({});
        auto index = d->model->index(d->model->rowCount() - 1,0);
        ui->treeView->selectionModel()->select(index,QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        ui->treeView->scrollTo(index);
        this->onFieldActivated(index);
    }else if(sender==ui->actionSave){
        auto driver = DBMSPane::getInstance()->connector(d->id);
        auto ret = driver->updateTableIndexes(d->name,d->indexes,d->model->indexes());
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
            if(MessageDialog::confirm(this,tr("Delete Index Confirm"),tr("Are you sure you want to delete the current index?"))==QMessageBox::Yes){
                d->model->removeItem(index.row());
            }
        }
    }
}

void TableIndexWidget::onIndexChanged(const QString& name,TableIndex* index){
    //d->modifications.insert(name,*field);
    //qDebug()<<"onIndexChanged"<<index->id<<index->name<<index->fields.size();
    d->model->updateItem(*index);

}

void TableIndexWidget::onFieldsChanged(const QStringList& fields){
    ui->form->fieldsChanged(fields);
}
}
