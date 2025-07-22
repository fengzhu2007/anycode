#include "workflow_item.h"
#include "image_process_thread.h"
#include "ui_workflow_item.h"
namespace ady{
class WorkflowItemPrivate{
public:
    int row;


};

WorkflowItem::WorkflowItem(QWidget *parent)
    : ListViewItem(parent)
    , ui(new Ui::WorkflowItem)
{
    ui->setupUi(this);
    d = new WorkflowItemPrivate;
    d->row = 0;
    connect(ui->remove,&QPushButton::clicked,this,&WorkflowItem::onRemove);
}


void WorkflowItem::init(const WorkflowData& data){
    ui->name->setText(data.title);
    QString params;
    if(data.name==ImageProcessThread::ProcessName::Scale){
        auto option = static_cast<ScaleOption*>(data.data);
        params = QString("Width:%1;Height:%2").arg(option->width).arg(option->height);
    }else if(data.name==ImageProcessThread::ProcessName::Resize){
        auto option = static_cast<ResizeOption*>(data.data);
        params = QString("Left:%1;Top:%2;Right:%3;Bottom:%4").arg(option->left).arg(option->top).arg(option->right).arg(option->bottom);
    }else if(data.name==ImageProcessThread::ProcessName::Cut){
        auto option = static_cast<CutOption*>(data.data);
        params = QString("Left:%1;Top:%2;Width:%5;Height:%6;Right:%3;Bottom:%4").arg(option->left).arg(option->top).arg(option->width).arg(option->height).arg(option->right).arg(option->bottom);
    }
    ui->params->setText(params);
}

void WorkflowItem::setRow(int row){
    d->row = row;
}


void WorkflowItem::onRemove(){
    emit removed(d->row);
}

WorkflowItem::~WorkflowItem()
{
    delete ui;
    delete d;
}
}
