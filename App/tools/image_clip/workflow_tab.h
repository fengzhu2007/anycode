#ifndef WORKFLOW_TAB_H
#define WORKFLOW_TAB_H

#include <QWidget>
#include "workflow_model.h"
namespace Ui {
class WorkflowTab;
}
namespace ady{
class WorkflowTabPrivate;
class WorkflowTab : public QWidget
{
    Q_OBJECT

public:
    explicit WorkflowTab(QWidget *parent = nullptr);
    ~WorkflowTab();
    void addFlow(const WorkflowData& item);
    QList<WorkflowData>& queue() const;//处理队列

private:
    Ui::WorkflowTab *ui;
    WorkflowTabPrivate* d;
};
}

#endif // WORKFLOW_TAB_H
