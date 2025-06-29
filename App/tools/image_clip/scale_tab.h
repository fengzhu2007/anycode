#ifndef SCALE_TAB_H
#define SCALE_TAB_H

#include <QWidget>

namespace Ui {
class ScaleTab;
}
namespace ady{
class ScaleTab : public QWidget
{
    Q_OBJECT

public:
    explicit ScaleTab(QWidget *parent = nullptr);
    ~ScaleTab();

    int optionWidth();
    int optionHeight();

private:
    Ui::ScaleTab *ui;
};
}

#endif // SCALE_TAB_H
