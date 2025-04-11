#ifndef FIELD_FORM_H
#define FIELD_FORM_H


#include "../field_form_impl.h"

namespace Ui {
class FieldForm;
}

namespace ady{

namespace sqlite{

class FieldForm : public FieldFormImpl
{
    Q_OBJECT

public:
    explicit FieldForm(QWidget *parent = nullptr);
    ~FieldForm();

    virtual void init(const TableField& field) override;
    virtual bool save() override;

private:
    Ui::FieldForm *ui;
};

}

}


#endif // FIELD_FORM_H
