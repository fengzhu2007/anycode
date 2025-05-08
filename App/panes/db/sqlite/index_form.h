#ifndef INDEX_FORM_H
#define INDEX_FORM_H

#include "../index_form_impl.h"

namespace Ui {
class IndexForm;
}

namespace ady{

namespace sqlite{
class IndexFormPrivate;
class IndexForm : public IndexFormImpl
{
    Q_OBJECT

public:
    explicit IndexForm(long long id,QWidget *parent = nullptr);
    ~IndexForm();

    virtual void init(const TableIndex& index) override;
    virtual bool save() override;
    virtual void fieldsChanged(const QStringList& fields) override;

public slots:
    void onValueChanged(int state);
    void onTextChanged(const QString& text);
    void onFieldChanged(const QModelIndex& index);


private:
    Ui::IndexForm *ui;
    IndexFormPrivate* d;
};

}

}


#endif // INDEX_FORM_H
