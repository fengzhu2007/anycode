#ifndef TABLE_SCHEME_WIDGET_H
#define TABLE_SCHEME_WIDGET_H

#include <QWidget>

namespace Ui {
class TableSchemeWidget;
}
namespace ady{
class TableSchemeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableSchemeWidget(QWidget *parent = nullptr);
    ~TableSchemeWidget();

private:
    Ui::TableSchemeWidget *ui;
};
}
#endif // TABLE_SCHEME_WIDGET_H
