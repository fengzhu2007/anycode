#ifndef INDEX_FORM_IMPL_H
#define INDEX_FORM_IMPL_H


#include "global.h"
#include <QWidget>

namespace ady{
class TableIndex;
class ANYENGINE_EXPORT IndexFormImpl : public QWidget
{
    Q_OBJECT
public:
    explicit IndexFormImpl(QWidget* parent);

    virtual void init(const TableIndex& index)=0;
    virtual bool save()=0;
    virtual void fieldsChanged(const QStringList& fields)=0;

signals:
    void change(const QString& name,TableIndex* current);

};
}


#endif // INDEX_FORM_IMPL_H
