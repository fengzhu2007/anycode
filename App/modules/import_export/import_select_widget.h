#ifndef IMPORT_SELECT_WIDGET_H
#define IMPORT_SELECT_WIDGET_H

#include <QWidget>

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

public slots:
    void onBrowser();

private:
    Ui::ImportSelectWidget *ui;
};
}
#endif // IMPORT_SELECT_WIDGET_H
