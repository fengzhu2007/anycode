#ifndef FIELD_FORM_H
#define FIELD_FORM_H


#include "../field_form_impl.h"

namespace Ui {
class FieldForm;
}

namespace ady{
namespace sqlite{
class FieldFormImplPrivate;
class FieldForm : public FieldFormImpl
{
    Q_OBJECT

public:
    explicit FieldForm(long long id,QWidget *parent = nullptr);
    ~FieldForm();

    void initView();

    virtual void init(const TableField& field) override;
    virtual bool save() override;


public slots:
    void onValueChanged(int state);
    void onTextChanged(const QString& text);
    void onTypeChanged(int index);

private:
    void setType(const QString& type);

private:
    Ui::FieldForm *ui;
    FieldFormImplPrivate* d;
};

}

}


#endif // FIELD_FORM_H
