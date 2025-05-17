#ifndef TABLE_SCHEME_WIDGET_H
#define TABLE_SCHEME_WIDGET_H

#include <QWidget>

namespace Ui {
class TableSchemeWidget;
}
namespace ady{
class TableSchemeWidgetPrivate;
class TableSchemeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableSchemeWidget(long long id,const QString& table,QWidget *parent = nullptr);
    ~TableSchemeWidget();

private:
    Ui::TableSchemeWidget *ui;
    TableSchemeWidgetPrivate* d;
};
}
#endif // TABLE_SCHEME_WIDGET_H
