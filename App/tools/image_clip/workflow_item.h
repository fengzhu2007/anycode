#ifndef WORKFLOW_ITEM_H
#define WORKFLOW_ITEM_H

#include "components/listview/listview_model.h"
#include "workflow_model.h"
namespace Ui {
class WorkflowItem;
}
namespace ady{
class WorkflowItemPrivate;
class WorkflowItem : public ListViewItem
{
    Q_OBJECT

public:
    explicit WorkflowItem(QWidget *parent = nullptr);
    ~WorkflowItem();
    virtual void init(const WorkflowData& data);
    void setRow(int row);

signals:
    void removed(int row);
public slots:
    void onRemove();

private:
    Ui::WorkflowItem *ui;
    WorkflowItemPrivate* d;
};
}
#endif // WORKFLOW_ITEM_H
