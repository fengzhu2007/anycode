#ifndef TABLE_FIELD_WIDGET_H
#define TABLE_FIELD_WIDGET_H

#include <QWidget>

namespace Ui {
class TableFieldWidget;
}

namespace ady{
class TableFieldWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableFieldWidget(QWidget *parent = nullptr);
    ~TableFieldWidget();

private:
    Ui::TableFieldWidget *ui;
};
}

#endif // TABLE_FIELD_WIDGET_H
