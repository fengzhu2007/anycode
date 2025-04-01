#ifndef TABLE_PANE_H
#define TABLE_PANE_H

#include <docking_pane.h>

namespace Ui {
class TablePane;
}

namespace ady{
class TablePanePrivate;
class TablePane : public DockingPane
{
    Q_OBJECT

public:
    explicit TablePane(QWidget *parent = nullptr);
    ~TablePane();
    void initView();
    virtual QString id() override;
    virtual QString group() override;


public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;
    //static const QString PANE_TITLE;

private:
    Ui::TablePane *ui;
    TablePanePrivate* d;
    static int SN;

};
}
#endif // TABLE_PANE_H
