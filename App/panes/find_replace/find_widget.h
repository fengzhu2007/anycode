#ifndef FIND_WIDGET_H
#define FIND_WIDGET_H

#include <QWidget>

namespace Ui {
class FindWidget;
}
namespace ady{
class FindWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FindWidget(QWidget *parent = nullptr);
    ~FindWidget();

public slots:
    void onClicked();
    void onSelectFolder();
private:
    Ui::FindWidget *ui;
};
}
#endif // FIND_WIDGET_H
