#ifndef REPLACE_WIDGET_H
#define REPLACE_WIDGET_H

#include <QWidget>

namespace Ui {
class ReplaceWidget;
}
namespace ady{
class ReplaceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReplaceWidget(QWidget *parent = nullptr);
    ~ReplaceWidget();

private:
    Ui::ReplaceWidget *ui;
};
}
#endif // REPLACE_WIDGET_H
