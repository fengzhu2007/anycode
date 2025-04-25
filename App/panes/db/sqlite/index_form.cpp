#include "index_form.h"
#include "ui_index_form.h"
#include "../table_field.h"
#include "components/select_model.h"

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
}

IndexForm::~IndexForm()
{
    delete d;
    delete ui;
}

void IndexForm::init(const TableIndex& index) {

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
}
}
