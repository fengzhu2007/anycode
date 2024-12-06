#ifndef NEW_PROJECT_TWO_WIDGET_H
#define NEW_PROJECT_TWO_WIDGET_H

#include <QWidget>

namespace Ui {
class NewProjectTwoWidget;
}

namespace ady{
class NewProjectTwoWidgetPrivate;
class NewProjectTwoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NewProjectTwoWidget(QWidget *parent = nullptr);
    ~NewProjectTwoWidget();
    void initData();
public slots:
    void onNewSite();
    void onSiteItemClicked(int i);
    void onTypeChanged(int i);
    void onAdjustHeight(int i);
    void onDeleteSite(int i);
    void onSave();
    void showSiteContextMenu(const QPoint &pos);
protected:
    virtual void paintEvent(QPaintEvent *e) override;
    virtual void showEvent(QShowEvent* e) override;
private:
    Ui::NewProjectTwoWidget *ui;
    NewProjectTwoWidgetPrivate *d;
};
}
#endif // NEW_PROJECT_TWO_WIDGET_H
