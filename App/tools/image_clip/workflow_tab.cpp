#include "workflow_tab.h"
#include "ui_workflow_tab.h"
#include "workflow_model.h"
namespace ady{

class WorkflowTabPrivate{
public:
    WorkflowModel* model;
};

WorkflowTab::WorkflowTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WorkflowTab)
{
    ui->setupUi(this);
    d = new WorkflowTabPrivate;
    d->model = new WorkflowModel(ui->listView);
    ui->listView->setModel(d->model);
    ui->listView->setStyleSheet("QScrollArea{border:0}");
}

WorkflowTab::~WorkflowTab()
{
    delete ui;
    delete d;
}

void WorkflowTab::addFlow(const WorkflowData& item){
    d->model->appendItem(item);
}

QList<WorkflowData>& WorkflowTab::queue() const{
    return d->model->dataSource();
}

}
