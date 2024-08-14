#ifndef REPLACE_WIDGET_H
#define REPLACE_WIDGET_H

#include <QWidget>

namespace Ui {
class ReplaceWidget;
}
namespace ady{
class ReplaceWidgetPrivate;
class ReplaceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReplaceWidget(QWidget *parent = nullptr);
    ~ReplaceWidget();
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
    Ui::ReplaceWidget *ui;
    ReplaceWidgetPrivate* d;
};
}
#endif // REPLACE_WIDGET_H
