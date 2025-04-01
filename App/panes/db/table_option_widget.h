#ifndef TABLE_OPTION_WIDGET_H
#define TABLE_OPTION_WIDGET_H

#include <QWidget>

namespace Ui {
class TableOptionWidget;
}

class TableOptionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableOptionWidget(QWidget *parent = nullptr);
    ~TableOptionWidget();

private:
    Ui::TableOptionWidget *ui;
};

#endif // TABLE_OPTION_WIDGET_H
