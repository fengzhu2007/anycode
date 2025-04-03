#ifndef TABLE_OPTION_WIDGET_H
#define TABLE_OPTION_WIDGET_H

#include <QWidget>

namespace Ui {
class TableOptionWidget;
}

namespace ady{
class TableOptionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableOptionWidget(long long id,const QString& table,QWidget *parent = nullptr);
    ~TableOptionWidget();

private:
    Ui::TableOptionWidget *ui;
};
}
#endif // TABLE_OPTION_WIDGET_H
