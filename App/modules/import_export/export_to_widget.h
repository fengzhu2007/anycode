#ifndef EXPORT_TO_WIDGET_H
#define EXPORT_TO_WIDGET_H

#include <QWidget>

namespace Ui {
class ExportToWidget;
}
namespace ady{
class ExportToWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ExportToWidget(QWidget *parent = nullptr);
    ~ExportToWidget();
    void initView();
    bool ok();

public slots:
    void onBrowser();

private:
    Ui::ExportToWidget *ui;
};
}

#endif // EXPORT_TO_WIDGET_H
