#ifndef TABLE_INDEX_WIDGET_H
#define TABLE_INDEX_WIDGET_H

#include <QWidget>

namespace Ui {
class TableIndexWidget;
}

class TableIndexWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableIndexWidget(QWidget *parent = nullptr);
    ~TableIndexWidget();

private:
    Ui::TableIndexWidget *ui;
};

#endif // TABLE_INDEX_WIDGET_H
