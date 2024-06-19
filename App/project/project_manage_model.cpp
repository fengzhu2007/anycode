#include "project_manage_model.h"
#include "components/listview/listview.h"
#include "new_project_zero_widget.h"
#include <QPushButton>
#include <QLabel>
#include <QStyleOption>
#include <QPainter>
#include <QDebug>
namespace ady{
class ProjectItemWidgetPrivate{
public:
    QLabel* icon;
    QLabel* title;
    QLabel* description;
    QPushButton* edit;
    int index = -1;
};

ProjectItemWidget::ProjectItemWidget(QWidget* parent)
    :ListViewItem(parent){
    this->setStyleSheet("ady--ProjectItemWidget QLabel#description{color:#999}"
                        "ady--ProjectItemWidget QPushButton{padding:0;background-color:transparent;border:1px solid transparent;}"
                        "ady--ProjectItemWidget QPushButton:hover{background-color:#e5f1fb;border:1px solid #007acc;}");
    d = new ProjectItemWidgetPrivate;
    QHBoxLayout* layout = new QHBoxLayout();
    QVBoxLayout* vLayout = new QVBoxLayout();
    layout->addLayout(vLayout,1);
    vLayout->setMargin(12);
    vLayout->setSpacing(6);
    this->setLayout(layout);
    d->title = new QLabel(this);
    d->description = new QLabel(this);
    d->edit = new QPushButton(this);
    d->description->setObjectName("description");
    d->edit->setIcon(QIcon(QString::fromUtf8(":/Resource/icons/CustomActionEditor_16x.svg")));
    vLayout->addWidget(d->title);
    vLayout->addWidget(d->description);
    layout->addWidget(d->edit);
    //qDebug()<<"parent:"<<parent->parentWidget()->parentWidget();
    //ListView* listView = (ListView*)parent->parentWidget()->parentWidget();
    connect(d->edit,&QPushButton::clicked,this,&ProjectItemWidget::onEditClicked);


}

ProjectItemWidget::~ProjectItemWidget(){
    delete d;
}

void ProjectItemWidget::setTitle(const QString& title){
    d->title->setText(title);
}

void ProjectItemWidget::setDescription(const QString& description){
    d->description->setText(description);
}

void ProjectItemWidget::setIndex(int i){
    d->index = i;
}

void ProjectItemWidget::onEditClicked(){
    emit editClicked(d->index);
}


/**************ProjectManageModel start******************/



class ProjectManageModelPrivate{
public:
    QList<ProjectRecord> list;
    ListView* listview;
};

ProjectManageModel::ProjectManageModel(ListView* parent)
    :ListViewModel(parent){
    d = new ProjectManageModelPrivate;
    d->listview = parent;
}

int ProjectManageModel::count(){
    return d->list.size();
}

ListViewItem* ProjectManageModel::item(int i){
    ProjectItemWidget* w = (ProjectItemWidget*)ListViewModel::item(i);
    if(w==nullptr){
        w = new ProjectItemWidget(d->listview->widget());
        ListViewModel::addWidget(w);
        NewProjectZeroWidget* widget = (NewProjectZeroWidget*)d->listview->parentWidget();
        connect(w,&ProjectItemWidget::editClicked,widget,&NewProjectZeroWidget::onProjectEditClicked);
    }
    ProjectRecord one = d->list.at(i);
    w->setTitle(one.name);
    w->setDescription(one.path);
    w->setIndex(i);
    return w;
}

void ProjectManageModel::setDataSource(QList<ProjectRecord> list){
    d->list = list;
    this->dataChanged();
}

ProjectRecord ProjectManageModel::itemAt(int i){
    return d->list.at(i);
}

}
