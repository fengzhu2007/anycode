#ifndef TABLE_FIELD_WIDGET_H
#define TABLE_FIELD_WIDGET_H

#include <QWidget>

namespace Ui {
class TableFieldWidget;
}

namespace ady{
class TableFieldWidgetPrivate;
class TableFieldWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableFieldWidget(long long id,const QString& table,QWidget *parent = nullptr);
    ~TableFieldWidget();

    void initData();



private:
    Ui::TableFieldWidget *ui;
    TableFieldWidgetPrivate* d;
};
}

#endif // TABLE_FIELD_WIDGET_H
