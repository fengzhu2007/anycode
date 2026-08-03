#ifndef RESIZE_TAB_H
#define RESIZE_TAB_H

#include <QWidget>

namespace Ui {
class ResizeTab;
}

namespace ady{
class ResizeTab : public QWidget
{
    Q_OBJECT

public:
    explicit ResizeTab(QWidget *parent = nullptr);
    ~ResizeTab();

    int optionLeft();
    int optionTop();
    int optionRight();
    int optionBottom();
    int optionWidth();
    int optionHeight();
    bool optionRelative();



private:
    Ui::ResizeTab *ui;
};
}
#endif // RESIZE_TAB_H
