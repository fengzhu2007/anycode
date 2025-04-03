#ifndef TABLE_LIST_WIDGET_H
#define TABLE_LIST_WIDGET_H

#include <QWidget>

namespace Ui {
class TableListWidget;
}


namespace ady{
class TableListWidgetPrivate;
class TableListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableListWidget(long long id,const QString& table,QWidget *parent = nullptr);
    ~TableListWidget();

    void initData();

protected:

private:
    Ui::TableListWidget *ui;
    TableListWidgetPrivate* d;
};
}
#endif // TABLE_LIST_WIDGET_H
