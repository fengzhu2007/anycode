#ifndef CUT_TAB_H
#define CUT_TAB_H

#include <QWidget>

namespace Ui {
class CutTab;
}
namespace ady{
class CutTab : public QWidget
{
    Q_OBJECT

public:
    explicit CutTab(QWidget *parent = nullptr);
    ~CutTab();
    int optionLeft();
    int optionTop();
    int optionRight();
    int optionBottom();
private:
    Ui::CutTab *ui;
};
}
#endif // CUT_TAB_H
