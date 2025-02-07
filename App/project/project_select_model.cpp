#include "project_select_model.h"
#include "components/listview/listview.h"
#include <QLabel>
#include <QHBoxLayout>

namespace ady{


class ProjectSelectItemWidgetPrivate{
public:
    QLabel* icon;
    QLabel* title;
    QLabel* description;
    int index = -1;
};

ProjectSelectItemWidget::ProjectSelectItemWidget(QWidget* parent)
    :ListViewItem(parent){
    this->setStyleSheet("ady--ProjectSelectItemWidget QLabel#description{color:#999}");
    d = new ProjectSelectItemWidgetPrivate;
    //QHBoxLayout* layout = new QHBoxLayout();
    QVBoxLayout* vLayout = new QVBoxLayout();
    ////layout->setMargin(0);
    //layout->setSpacing(0);
    //layout->addLayout(vLayout,1);
    vLayout->setMargin(12);
    vLayout->setSpacing(6);
    this->setLayout(vLayout);
    d->title = new QLabel(this);
    d->description = new QLabel(this);
    d->description->setObjectName("description");
    vLayout->addWidget(d->title);
    vLayout->addWidget(d->description);
}

ProjectSelectItemWidget::~ProjectSelectItemWidget(){
    delete d;
}

void ProjectSelectItemWidget::setTitle(const QString& title){
    d->title->setText(title);
}

void ProjectSelectItemWidget::setDescription(const QString& description){
    d->description->setText(description);
}

void ProjectSelectItemWidget::setIndex(int i){
    d->index = i;
}




ProjectSelectModel::ProjectSelectModel(ListView* parent)
    :ProjectManageModel(parent)
{

}

ListViewItem* ProjectSelectModel::item(int i){

    ProjectSelectItemWidget* w = (ProjectSelectItemWidget*)ListViewModel::item(i);
     qDebug()<<"item i"<<i<<w<<this;
    if(w==nullptr){
        w = new ProjectSelectItemWidget(this->listView()->widget());
        ListViewModel::addWidget(w);
    }
    ProjectRecord one = this->itemAt(i);
    w->setTitle(one.name);
    w->setDescription(one.path);
    w->setIndex(i);
    return w;
}

}
