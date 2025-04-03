#ifndef TABLE_INDEX_WIDGET_H
#define TABLE_INDEX_WIDGET_H

#include <QWidget>

namespace Ui {
class TableIndexWidget;
}

namespace ady{
class TableIndexWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableIndexWidget(long long id,const QString& table,QWidget *parent = nullptr);
    ~TableIndexWidget();

private:
    Ui::TableIndexWidget *ui;
};
}
#endif // TABLE_INDEX_WIDGET_H
