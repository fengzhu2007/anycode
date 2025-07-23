#include "workflow_model.h"
#include "workflow_item.h"
#include "image_process_thread.h"
#include "components/listview/listview.h"
#include <QDebug>

namespace ady{


WorkflowData::WorkflowData(const WorkflowData& other){
    //qDebug()<<"copy =:"<<this<<&other;
    this->name = other.name;
    this->title = other.title;
    if(name==ImageProcessThread::ProcessName::Scale){
        this->scale = other.scale;
    }else if(name==ImageProcessThread::ProcessName::Resize){
        this->resize = other.resize;
    }else if(name==ImageProcessThread::ProcessName::Cut){
        this->cut = other.cut;
    }
}

WorkflowData& WorkflowData::operator=(const WorkflowData& other){
    //qDebug()<<"operator=:"<<this<<&other;
    this->name = other.name;
    this->title = other.title;
    if(name==ImageProcessThread::ProcessName::Scale){
        this->scale = other.scale;
    }else if(name==ImageProcessThread::ProcessName::Resize){
        this->resize = other.resize;
    }else if(name==ImageProcessThread::ProcessName::Cut){
        this->cut = other.cut;
    }
    return *this;
}

bool WorkflowData::operator==(const WorkflowData& other) const{
    qDebug()<<"WorkflowData::operator==";
     qDebug()<<"name"<<this->name<<";other:name"<<other.name;
     qDebug()<<"title"<<this->title<<";other:name"<<other.title;
    if(this->name==other.name && this->title==other.title){
        qDebug()<<"name"<<this->name<<";other:name"<<other.name;
        if(name==ImageProcessThread::ProcessName::Scale){
            return this->scale==other.scale;
        }else if(name==ImageProcessThread::ProcessName::Resize){
            return this->resize == other.resize;
        }else if(name==ImageProcessThread::ProcessName::Cut){
            return this->cut == other.cut;
        }
        return false;
    }else{
        return false;
    }
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
    //qDebug()<<"workflow model item:"<<i;
    auto w = static_cast<WorkflowItem*>(ListViewModel::item(i));
    if(w==nullptr){
        w = new WorkflowItem(d->listview->widget());
        ListViewModel::addWidget(w);
        connect(w,&WorkflowItem::removed,this,&WorkflowModel::onRemoved);
    }
    WorkflowData one = d->list.at(i);
    w->init(one);
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

void WorkflowModel::onRemoved(){
    auto sender = static_cast<WorkflowItem*>(this->sender());
    auto data = sender->data();

    auto i = 0;
    for(auto v:d->list){
        if(data==v){
            this->itemRemoved(i);
            return ;
        }
        i++;
    }
}

QList<WorkflowData>& WorkflowModel::dataSource(){
    return d->list;
}

}






