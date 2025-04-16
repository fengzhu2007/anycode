#include "field_form.h"
#include "ui_field_form.h"
#include "../table_field.h"
namespace ady{

namespace sqlite{
class FieldFormImplPrivate{
public:
    TableField field;
    TableField current;
};

FieldForm::FieldForm(QWidget *parent)
    : FieldFormImpl(parent)
    , ui(new Ui::FieldForm)
{
    ui->setupUi(this);
    d = new FieldFormImplPrivate;
    connect(ui->name,&QLineEdit::textChanged,this,&FieldForm::onTextChanged);
    connect(ui->type,&QComboBox::currentTextChanged,this,&FieldForm::onTextChanged);
    connect(ui->defaultValue,&QComboBox::currentTextChanged,this,&FieldForm::onTextChanged);
    connect(ui->length,QOverload<int>::of(&QSpinBox::valueChanged),this,&FieldForm::onValueChanged);
    connect(ui->decimal,QOverload<int>::of(&QSpinBox::valueChanged),this,&FieldForm::onValueChanged);
    connect(ui->notNull,&QCheckBox::stateChanged,this,&FieldForm::onValueChanged);
    connect(ui->primaryKey,&QCheckBox::stateChanged,this,&FieldForm::onValueChanged);
    connect(ui->autoIncrement,&QCheckBox::stateChanged,this,&FieldForm::onValueChanged);
}

FieldForm::~FieldForm()
{
    delete d;
    delete ui;
}



void FieldForm::init(const TableField& field){

    d->field = field;
    d->current = field;

    ui->name->setText(field.name);
    ui->type->setCurrentText(field.type);
    ui->length->setValue(field.length);
    ui->decimal->setValue(field.decimal);

    ui->defaultValue->setCurrentText(field.defaultValue);
    ui->notNull->setChecked(field.notNull);
    ui->primaryKey->setChecked(field.primaryKey);
    ui->autoIncrement->setChecked(field.autoIncrement);

}

bool FieldForm::save(){

    return true;
}

void FieldForm::onValueChanged(int value){
    auto sender = this->sender();
    if(sender==ui->notNull){
        d->current.notNull = value==Qt::Checked;
    }else if(sender==ui->primaryKey){
        d->current.primaryKey = value==Qt::Checked;
    }else if(sender==ui->autoIncrement){
        d->current.autoIncrement = value==Qt::Checked;
    }else if(sender==ui->length){
        d->current.length = value;
    }else if(sender==ui->decimal){
        d->current.decimal = value;
    }
    emit change(d->field.name,&d->current);
}

void FieldForm::onTextChanged(const QString& text){
    auto sender = this->sender();
    if(sender==ui->name){
        d->current.name = text.trimmed();
    }else if(sender==ui->type){
        d->current.type = text.trimmed();
    }else if(sender==ui->defaultValue){
        d->current.defaultValue = text.trimmed();
    }
    emit change(d->field.name,&d->current);
}

}}
