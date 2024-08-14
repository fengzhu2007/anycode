#ifndef FIND_WIDGET_H
#define FIND_WIDGET_H

#include <QWidget>

namespace Ui {
class FindWidget;
}
namespace ady{
class FindWidgetPrivate;
class FindWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FindWidget(QWidget *parent = nullptr);
    ~FindWidget();
    void clear();

    void setFindText(int i);
    void setSearchScope(int i);

public slots:
    void onClicked();
    void onSelectFolder();
    void onSearchScopeChanged(int i);

protected:
    void showEvent(QShowEvent* e);


private:
    Ui::FindWidget *ui;
    FindWidgetPrivate* d;
};
}
#endif // FIND_WIDGET_H
