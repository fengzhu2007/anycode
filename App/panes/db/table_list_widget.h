#ifndef TABLE_LIST_WIDGET_H
#define TABLE_LIST_WIDGET_H

#include <QWidget>

namespace Ui {
class TableListWidget;
}


namespace ady{
class TableListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableListWidget(QWidget *parent = nullptr);
    ~TableListWidget();

private:
    Ui::TableListWidget *ui;
};
}
#endif // TABLE_LIST_WIDGET_H
