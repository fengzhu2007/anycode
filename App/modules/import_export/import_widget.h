#ifndef IMPORT_WIDGET_H
#define IMPORT_WIDGET_H

#include <QWidget>

namespace Ui {
class ImportWidget;
}
namespace ady{
class SelectTreeModel;
class ImportWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImportWidget(QWidget *parent = nullptr);
    ~ImportWidget();
    void initView();
    bool ok();


public slots:
    void onBrowser();

private:
    SelectTreeModel* m_model;
    Ui::ImportWidget *ui;
};
}
#endif // IMPORT_WIDGET_H
