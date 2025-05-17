#include "field_form.h"
#include "ui_field_form.h"
#include "../table_field.h"
#include "../db_driver.h"
#include "../dbms_pane.h"
#include "components/select_model.h"
#include "components/list_item_delegate.h"
namespace ady{

namespace sqlite{
class FieldFormImplPrivate{
public:
    long long id;
    TableField field;
    TableField current;
    QList<QPair<QString,int>> fieldTypes;
    SelectModel<QString>* model;
};

FieldForm::FieldForm(long long id,QWidget *parent)
    : FieldFormImpl(parent)
    , ui(new Ui::FieldForm)
{
    ui->setupUi(this);
    d = new FieldFormImplPrivate;
    d->id = id;

    d->model = new SelectModel<QString>(ui->type);
    ui->type->setModel(d->model);
    ui->type->setItemDelegate(new ListItemDelegate(ui->type));

    connect(ui->name,&QLineEdit::textChanged,this,&FieldForm::onTextChanged);
    connect(ui->type,&QComboBox::currentTextChanged,this,&FieldForm::onTextChanged);
    connect(ui->type,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&FieldForm::onTypeChanged);
    connect(ui->defaultValue,&QComboBox::currentTextChanged,this,&FieldForm::onTextChanged);
    connect(ui->length,QOverload<int>::of(&QSpinBox::valueChanged),this,&FieldForm::onValueChanged);
    connect(ui->decimal,QOverload<int>::of(&QSpinBox::valueChanged),this,&FieldForm::onValueChanged);
    connect(ui->notNull,&QCheckBox::stateChanged,this,&FieldForm::onValueChanged);
    connect(ui->primaryKey,&QCheckBox::stateChanged,this,&FieldForm::onValueChanged);
    connect(ui->autoIncrement,&QCheckBox::stateChanged,this,&FieldForm::onValueChanged);

    this->initView();

}

FieldForm::~FieldForm()
{
    delete d;
    delete ui;
}

void FieldForm::initView(){

    auto driver = DBMSPane::getInstance()->connector(d->id);
    if(driver){
        d->fieldTypes = driver->fieldTypes();
        QList<QPair<QString,QString>>list;
        list.append(QPair{"",""});
        for(auto one:d->fieldTypes){
            list.append({one.first,one.first});
        }
        d->model->setDataSource(list);
    }
}



void FieldForm::init(const TableField& field){
    if(field.id==d->current.id){
        return ;
    }
    d->field = field;
    d->current = field;
    ui->name->setFocus();
    ui->name->setText(field.name);
    //ui->type->setCurrentText(field.type);
    this->setType(field.type);
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

void FieldForm::onTypeChanged(int index){
    auto value = d->model->value(index);
    this->setType(value);
    auto item = d->fieldTypes.at(index);
    if(item.second==0){
        ui->length->setEnabled(false);
        ui->decimal->setEnabled(false);
        emit ui->length->valueChanged(d->current.length);
        emit ui->decimal->valueChanged(d->current.decimal);
    }else if(item.second==1){
        ui->length->setEnabled(true);
        ui->decimal->setEnabled(false);
        emit ui->decimal->valueChanged(d->current.decimal);
    }else if(item.second==2){
        ui->length->setEnabled(true);
        ui->decimal->setEnabled(true);
    }
}

void FieldForm::setType(const QString& type){
    for(int i=0;i<d->fieldTypes.length();i++){
        auto one = d->fieldTypes.at(i);
        if(type==one.first){
            ui->type->setCurrentIndex(i);
            ui->type->setCurrentText(type);
            return ;
        }
    }
    ui->type->setCurrentText(type);
}

}}
