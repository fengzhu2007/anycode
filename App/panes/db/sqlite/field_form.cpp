#include "field_form.h"
#include "ui_field_form.h"
namespace ady{

namespace sqlite{
FieldForm::FieldForm(QWidget *parent)
    : FieldFormImpl(parent)
    , ui(new Ui::FieldForm)
{
    ui->setupUi(this);
}

FieldForm::~FieldForm()
{
    delete ui;
}


void FieldForm::init(const TableField& field){

}

bool FieldForm::save(){

    return true;
}

}}
