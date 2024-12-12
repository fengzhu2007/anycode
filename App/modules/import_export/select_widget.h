#ifndef SELECT_WIDGET_H
#define SELECT_WIDGET_H

#include <QWidget>

namespace Ui {
class SelectWidget;
}

namespace ady{
class SelectWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SelectWidget(QWidget *parent = nullptr);
    ~SelectWidget();
    int current();
    int result();

private:
    Ui::SelectWidget *ui;
};
}

#endif // SELECT_WIDGET_H
