#ifndef EXPORT_WIDGET_H
#define EXPORT_WIDGET_H

#include <QWidget>

namespace Ui {
class ExportWidget;
}
namespace ady{
class SelectTreeModelItem;
class ExportWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ExportWidget(QWidget *parent = nullptr);
    ~ExportWidget();
    void initView();
    QList<SelectTreeModelItem*> result();


public slots:
    void onStateChanged(int state);

private:
    Ui::ExportWidget *ui;
};
}
#endif // EXPORT_WIDGET_H
