#include "workflow_item.h"
#include "image_process_thread.h"
#include "ui_workflow_item.h"
#include <QDebug>
namespace ady{
class WorkflowItemPrivate{
public:
   // int row;
    WorkflowData data;

};

WorkflowItem::WorkflowItem(QWidget *parent)
    : ListViewItem(parent)
    , ui(new Ui::WorkflowItem)
{
    ui->setupUi(this);
    d = new WorkflowItemPrivate;
    //d->row = 0;
    connect(ui->remove,&QPushButton::clicked,this,&WorkflowItem::removed);
}


void WorkflowItem::init(const WorkflowData& data){
    d->data = data;
    ui->name->setText(data.title);
    QString params;
    if(data.name==ImageProcessThread::ProcessName::Scale){
        auto option = data.scale;
        params = QString("Width:%1;Height:%2").arg(option.width).arg(option.height);
    }else if(data.name==ImageProcessThread::ProcessName::Resize){
        auto option = data.resize;
        params = QString("Left:%1;Top:%2;Right:%3;Bottom:%4").arg(option.left).arg(option.top).arg(option.right).arg(option.bottom);
    }else if(data.name==ImageProcessThread::ProcessName::Cut){
        auto option = data.cut;
        params = QString("Left:%1;Top:%2;Width:%3;Height:%4;Right:%5;Bottom:%6").arg(option.left).arg(option.top)
                     .arg(option.width).arg(option.height).arg(option.right).arg(option.bottom);
    }
    ui->params->setText(params);
}


WorkflowData& WorkflowItem::data()const{
    return d->data;
}


WorkflowItem::~WorkflowItem()
{
    delete ui;
    delete d;
}
}
