#ifndef NEW_PROJECT_ONE_WIDGET_H
#define NEW_PROJECT_ONE_WIDGET_H

#include <QWidget>

namespace Ui {
class NewProjectOneWidget;
}

namespace ady{
class NewProjectOneWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NewProjectOneWidget(QWidget *parent = nullptr);
    ~NewProjectOneWidget();
    void initData();

public slots:
    void onNext();
    void onSave();

protected:
    virtual void paintEvent(QPaintEvent *e) override;
private:
    Ui::NewProjectOneWidget *ui;
};
}
#endif // NEW_PROJECT_ONE_WIDGET_H
