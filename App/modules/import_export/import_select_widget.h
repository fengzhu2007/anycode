#ifndef IMPORT_SELECT_WIDGET_H
#define IMPORT_SELECT_WIDGET_H

#include <QWidget>
#include <QAbstractButton>

namespace Ui {
class ImportSelectWidget;
}

namespace ady{
class ImportSelectWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImportSelectWidget(QWidget *parent = nullptr);
    ~ImportSelectWidget();
    void initView();
    int result();
    QString filename();
    QString directory();

public slots:
    void onBrowser();
    void onToggle(QAbstractButton *button);

private:
    Ui::ImportSelectWidget *ui;
};
}
#endif // IMPORT_SELECT_WIDGET_H
