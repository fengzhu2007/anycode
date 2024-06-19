#include "new_project_two_widget.h"
#include "ui_new_project_two_widget.h"
#include "new_project_window.h"
#include "storage/SiteStorage.h"
#include "storage/AddonStorage.h"
#include "storage/GroupStorage.h"
#include "site_list_model.h"
#include "SiteGroupListModel.h"
#include "SiteTypeListModel.h"
#include "AddonLoader.h"
#include "interface/FormPanel.h"
#include <QStyleOption>
#include <QPainter>
#include <QPushButton>
#include <QList>
namespace ady{
class NewProjectTwoWidgetPrivate{
public:
    QList<AddonRecord> addons;
    QList<GroupRecord> groups;
    SiteRecord current;
    QList<FormPanel*> panels;
};

NewProjectTwoWidget::NewProjectTwoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NewProjectTwoWidget)
{
    d = new NewProjectTwoWidgetPrivate;
    this->setStyleSheet(QString::fromUtf8("QTabWidget::pane{border:0;border-top:1px solid #ccc}"));
    ui->setupUi(this);

    NewProjectWindow* window = (NewProjectWindow*)this->parentWidget()->parentWidget();
    connect(ui->newSite,&QPushButton::clicked,this,&NewProjectTwoWidget::onNewSite);
    connect(ui->previous,&QPushButton::clicked,window,&NewProjectWindow::previous);
    connect(ui->listView,&ListView::itemClicked,this,&NewProjectTwoWidget::onSiteItemClicked);
    connect(ui->type,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&NewProjectTwoWidget::onTypeChanged);

    AddonStorage addonStorage;
    d->addons = addonStorage.list(1);

    //fill combobox
    auto model = new SiteTypeListModel(ui->type);
    model->setData(d->addons);
    ui->type->setModel(model);
    ui->group->setModel(new SiteGroupListModel(ui->group));

    //tab first tab widget

}

NewProjectTwoWidget::~NewProjectTwoWidget()
{
    delete ui;
}

void NewProjectTwoWidget::initData(){
    NewProjectWindow* window = (NewProjectWindow*)parentWidget()->parentWidget();
    long long id = window->currentProjectId();
    //id = 1l;
    //search site list;
    SiteStorage siteStorage;
    QList<SiteRecord> list = siteStorage.list(id);
    SiteListModel* model = (SiteListModel*)ui->listView->model();
    if(model==nullptr){
        model = new SiteListModel(ui->listView);
        model->setDataSource(list);
        ui->listView->setModel(model);
    }else{
        model->setDataSource(list);
    }
    {
        GroupStorage groupStorage;
        d->groups = groupStorage.list(id);
        auto model = static_cast<SiteGroupListModel*>(ui->group->model());
        model->setData(d->groups);
    }

}

void NewProjectTwoWidget::onNewSite(){
    ui->name->clear();
    ui->type->setCurrentIndex(-1);
    ui->group->setCurrentIndex(-1);
}

void NewProjectTwoWidget::onSiteItemClicked(int i){
    SiteListModel* model = (SiteListModel*)ui->listView->model();
    d->current = model->itemAt(i);
    ui->name->setText(d->current.name);
    for(int i=0;i<d->addons.length();i++){
        if(d->addons[i].name==d->current.type){
            ui->type->setCurrentIndex(i);
            break;
        }
    }
    for(int i=0;i<d->groups.length();i++){
        if(d->groups[i].id==d->current.groupid){
            ui->group->setCurrentIndex(i);
            break;
        }
    }

    if(d->current.id>0){
        //init form data
        QList<FormPanel*>::iterator iter = d->panels.begin();
        while(iter!=d->panels.end()){
            (*iter)->initFormData(d->current);
            iter++;
        }
        ui->status->setChecked(d->current.status==1?true:false);
    }
}

void NewProjectTwoWidget::onTypeChanged(int i){
    //clear old forms
    ui->tabWidget->clear();
    foreach(auto one,d->panels){
        one->close();
        one->deleteLater();
    }
    d->panels.clear();
    if(i>=0){
        AddonRecord one = d->addons.at(i);
        QString name = one.name;
        AddonLoader* loader = AddonLoader::getInstance();
        bool ret = loader->load(one.file);
        if(ret){
            int size =loader->getFormPanelSize(name);
            for(int i=0;i<size;i++){
                FormPanel* panel = loader->getFormPanel(i==0 ? ((QWidget*)this):ui->tabWidget,name,i);
                if(panel!=nullptr){
                    d->panels.push_back(panel);
                    ui->tabWidget->addTab(panel,panel->windowTitle());
                }
            }
            if(d->current.id>0){
                //init form data
                QList<FormPanel*>::iterator iter = d->panels.begin();
                while(iter!=d->panels.end()){
                    (*iter)->initFormData(d->current);
                    iter++;
                }
            }
        }else{
            //Toast
        }
    }

}

void NewProjectTwoWidget::paintEvent(QPaintEvent *e){
    Q_UNUSED(e);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

}
