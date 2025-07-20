#include "workflow_model.h"
#include "workflow_item.h"
#include "image_process_thread.h"
#include "components/listview/listview.h"
#include <QDebug>

namespace ady{

WorkflowData::~WorkflowData(){
    if(this->data!=nullptr){
        if(name==ImageProcessThread::ProcessName::Scale){
            delete static_cast<ScaleOption*>(data);
        }else if(name==ImageProcessThread::ProcessName::Resize){
            delete static_cast<ResizeOption*>(data);
        }else if(name==ImageProcessThread::ProcessName::Cut){
            delete static_cast<CutOption*>(data);
        }
    }

}

WorkflowData::WorkflowData(const WorkflowData& other){
    this->name = other.name;
    this->title = other.title;
    if(name==ImageProcessThread::ProcessName::Scale){
        auto option = static_cast<ScaleOption*>(other.data);
        this->data = new ScaleOption{option->width,option->height};
    }else if(name==ImageProcessThread::ProcessName::Resize){
        auto option = static_cast<ResizeOption*>(other.data);
        this->data = new ResizeOption{option->left,option->top,option->right,option->bottom,true};
    }else if(name==ImageProcessThread::ProcessName::Cut){
        auto option = static_cast<CutOption*>(other.data);
        this->data = new CutOption{option->left,option->top,option->right,option->bottom};
    }else{
        this->data = nullptr;
    }

}

WorkflowData& WorkflowData::operator=(const WorkflowData& other){
    this->name = other.name;
    this->title = other.title;
    if(name==ImageProcessThread::ProcessName::Scale){
        auto option = static_cast<ScaleOption*>(other.data);
        this->data = new ScaleOption{option->width,option->height};
    }else if(name==ImageProcessThread::ProcessName::Resize){
        auto option = static_cast<ResizeOption*>(other.data);
        this->data = new ResizeOption{option->left,option->top,option->right,option->bottom,true};
    }else if(name==ImageProcessThread::ProcessName::Cut){
        auto option = static_cast<CutOption*>(other.data);
        this->data = new CutOption{option->left,option->top,option->right,option->bottom};
    }else{
        this->data = nullptr;
    }
    return *this;
}




class WorkflowModelPrivate{
public:
    QList<WorkflowData> list;
    ListView* listview;
};

WorkflowModel::WorkflowModel(ListView* parent)
    :ListViewModel(parent){
    d = new WorkflowModelPrivate;
    d->listview = parent;
}

int WorkflowModel::count(){
    return d->list.size();
}

ListViewItem* WorkflowModel::item(int i){
    qDebug()<<"workflow model item:"<<i;
    auto w = static_cast<WorkflowItem*>(ListViewModel::item(i));
    if(w==nullptr){
        w = new WorkflowItem(d->listview->widget());
        ListViewModel::addWidget(w);
        connect(w,&WorkflowItem::removed,this,&WorkflowModel::onRemoved);
    }
    WorkflowData one = d->list.at(i);
    w->init(one);
    w->setRow(i);
    return w;
}

void WorkflowModel::itemRemoved(int i){
    d->list.takeAt(i);
    ListViewModel::itemRemoved(i);
}

void WorkflowModel::setDataSource(QList<WorkflowData> list){
    d->list = list;
    this->dataChanged();
}

void WorkflowModel::appendItem(const WorkflowData& item){
    d->list.append(item);
    this->dataChanged();
}

WorkflowData WorkflowModel::itemAt(int i){
    return d->list.at(i);
}

void WorkflowModel::onRemoved(int i){
    this->itemRemoved(i);
}

QList<WorkflowData>& WorkflowModel::dataSource(){
    return d->list;
}

}






