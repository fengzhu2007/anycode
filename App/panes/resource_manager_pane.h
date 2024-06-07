#ifndef RESOURCE_MANAGER_PANE_H
#define RESOURCE_MANAGER_PANE_H

#include "docking_pane.h"

namespace Ui {
class ResourceManagerPane;
}

namespace ady{

class ResourceManagerPane : public DockingPane
{
    Q_OBJECT

public:
    explicit ResourceManagerPane(QWidget *parent = nullptr);
    ~ResourceManagerPane();
    virtual QString id() override;
    virtual QString group() override;

private:
    Ui::ResourceManagerPane *ui;
};

}
#endif // RESOURCE_MANAGER_PANE_H
