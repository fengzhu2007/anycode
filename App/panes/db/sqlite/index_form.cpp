#include "index_form.h"
#include "ui_index_form.h"
#include "../table_field.h"
#include "components/select_model.h"
#include <QDebug>

namespace ady{
namespace sqlite{

class IndexFormPrivate{
public:
    long long id;
    TableIndex index;
    TableIndex current;
    QList<QPair<QString,int>> fieldTypes;
    SelectModel<QString>* model;

};

IndexForm::IndexForm(long long id,QWidget *parent)
    : IndexFormImpl(parent)
    , ui(new Ui::IndexForm)
{
    ui->setupUi(this);
    d = new IndexFormPrivate;
    d->id = id;
    d->model = new SelectModel<QString>(ui->listView);
    ui->listView->setModel(d->model);

    connect(ui->name,&QLineEdit::textChanged,this,&IndexForm::onTextChanged);
    connect(ui->uniqueKey,&QCheckBox::stateChanged,this,&IndexForm::onValueChanged);
    connect(ui->listView,&QListView::clicked,this,&IndexForm::onFieldChanged);

}

IndexForm::~IndexForm()
{
    delete d;
    delete ui;
}

void IndexForm::init(const TableIndex& index) {
    if(index.id==d->current.id){
        return ;
    }
    d->index = index;
    d->current = index;
    ui->name->setFocus();
    ui->name->setText(index.name);
    //qDebug()<<"isUnique"<<index.isUnique;
    ui->uniqueKey->setChecked(index.isUnique);
    auto selectionModel = ui->listView->selectionModel();
    selectionModel->clearSelection();
    if(index.fields.length()>0){

        for(auto one:index.fields){
            int row = d->model->indexOf(one.name);
            if(row!=-1){
                auto index = d->model->index(row,0);
                selectionModel->select(index,QItemSelectionModel::Select);
                ui->listView->scrollTo(index);
            }

        }
    }
}

bool IndexForm::save() {

    return false;
}

void IndexForm::fieldsChanged(const QStringList& fields){
    QList<QPair<QString,QString>>list;
    for(auto one:fields){
        list.append({one,one});
    }
    d->model->setDataSource(list);
}

void IndexForm::onValueChanged(int value){
    auto sender = this->sender();
    if(sender==ui->uniqueKey){
        d->current.isUnique = value==Qt::Checked;
    }
    emit change(d->index.name,&d->current);
}

void IndexForm::onTextChanged(const QString& text){
    auto sender = this->sender();
    if(sender==ui->name){
        d->current.name = text.trimmed();
    }
    emit change(d->index.name,&d->current);
}

void IndexForm::onFieldChanged(const QModelIndex& index){
    auto indexlist = ui->listView->selectionModel()->selectedIndexes();
    QList<TableIndexColumn> fields;
    for(auto index:indexlist){
        auto val = d->model->value(index.row());
        fields.append({val});
    }
    d->current.fields = fields;
    emit change(d->index.name,&d->current);
}


}
}
