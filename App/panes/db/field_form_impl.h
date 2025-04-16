#ifndef FIELD_FORM_IMPL_H
#define FIELD_FORM_IMPL_H

#include "global.h"
#include <QWidget>

namespace ady{
class TableField;
class ANYENGINE_EXPORT FieldFormImpl : public QWidget
{
    Q_OBJECT
public:
    explicit FieldFormImpl(QWidget* parent);

    virtual void init(const TableField& field)=0;
    virtual bool save()=0;

signals:
    void change(const QString& name,TableField* current);

};
}

#endif // FIELD_FORM_IMPL_H
