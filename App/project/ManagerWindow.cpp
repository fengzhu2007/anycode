#include "ManagerWindow.h"
#include "ui_ManagerWindow.h"
#include "storage/project_storage.h"
#include "storage/site_storage.h"
#include "storage/group_storage.h"
#include "ManagerTreeModel.h"
#include "ManagerTreeItem.h"
#include <QSplitterHandle>
#include "SplitterHandleWidget.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QDateTime>
#include "ProjectEditWidget.h"
#include "SiteFrameworkWidget.h"
#include "components/message_dialog.h"
#include "open_project_window.h"

#include <QDebug>
namespace ady{

ManagerWindow::ManagerWindow(QWidget* parent)
    :QDialog(parent),
    ui(new Ui::ManagerWindow)
{
    ui->setupUi(this);
    this->model = nullptr;
    this->initView();
    this->initData();

}

void ManagerWindow::initView(){
    ui->stackedWidget->setCurrentIndex(0);


    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 2);

    //ui->splitter->setHandleWidth(30);
    QSplitterHandle* handle = ui->splitter->handle(1);
    handle->setCursor(QCursor(Qt::ArrowCursor));

    QVBoxLayout *lt = new QVBoxLayout();
    lt->setSpacing(0);
    lt->setMargin(0);
    lt->addWidget(this->handle = new SplitterHandleWidget(handle));
    handle->setLayout(lt);

    ui->projectTabWidget->addTab(new ProjectEditWidget(ui->projectTabWidget),tr("General"));

    ui->siteTabWidget->addTab(new SiteFrameworkWidget(ui->siteTabWidget),tr("General"));


    connect(ui->projectSaveButton,&QPushButton::clicked,this,&ManagerWindow::onProjectSave);
    connect(ui->siteSaveButton,&QPushButton::clicked,this,&ManagerWindow::onSiteSave);
    connect(ui->connectButton,&QPushButton::clicked,this,&ManagerWindow::onSiteTest);

    connect(ui->treeView,&QTreeView::clicked,this,&ManagerWindow::onTreeItemClicked);


    connect(this->handle,SIGNAL(actionTriggered(QAction*,bool)),this,SLOT(onTriggered(QAction* ,bool)));

}

void ManagerWindow::initData()
{

    QList<ManagerTreeItem*> lists;
    if(this->model==nullptr){
        this->model = new ManagerTreeModel(lists,ui->treeView);
        ui->treeView->setModel(this->model);
        ui->treeView->setColumnWidth(0,160);
    }

    ProjectStorage projectStorage;
    QList<ProjectRecord>projects = projectStorage.all();

    GroupStorage groupStorage;
    QList<GroupRecord>groups = groupStorage.all();

    SiteStorage siteStorage;
    QList<SiteRecord> sites = siteStorage.all();
    ManagerTreeItem* rootItem = this->model->getRootItem();
    for(int i=0;i<projects.length();i++){
        ManagerTreeData data(ManagerTreeData::PROJECT,projects[i].id,projects[i].name,projects[i].path);
        ManagerTreeItem *item = new ManagerTreeItem(data,rootItem);
        for(int j=0;j<groups.length();j++){
            if(groups[j].pid==projects[i].id || groups[j].pid==0){
                ManagerTreeData data(ManagerTreeData::GROUP,groups[j].id,groups[j].name,"");
                ManagerTreeItem *subItem = new ManagerTreeItem(data,item);
                item->appendChild(subItem);
                for(int k=0;k<sites.length();k++){
                    if(sites[k].pid==projects[i].id && sites[k].groupid==groups[j].id){
                        ManagerTreeData data(ManagerTreeData::SITE,sites[k].id,sites[k].name,sites[k].path);
                        ManagerTreeItem *thirdItem = new ManagerTreeItem(data,subItem);
                        subItem->appendChild(thirdItem);
                    }
                }
            }
        }
        lists.push_back(item);
    }

    for(int i=0;i<sites.length();i++){
        if(sites[i].pid==0){
            ManagerTreeData data(ManagerTreeData::SITE,sites[i].id,sites[i].name,sites[i].path);
            ManagerTreeItem *item = new ManagerTreeItem(data);
            lists.push_back(item);
        }
    }
    this->model->setData(lists);
    ui->treeView->expandAll();
}

void ManagerWindow::onProjectSave()
{
    ProjectEditWidget* tab = dynamic_cast<ProjectEditWidget*>(ui->projectTabWidget->widget(0));
    ProjectRecord record;
    tab->getFormData(record);
    ProjectStorage projectStorage;
    bool saveState = false;
    long long id = 0l;
    if(record.id>0){
        bool ret = projectStorage.update(record);
        saveState = ret;
    }else{
        record.datetime = QDateTime::currentDateTime().currentSecsSinceEpoch();
        id = projectStorage.insert(record);
        if(id>0){
            record.id = id;
            tab->setFormData(record);
        }
        saveState = id>0;
    }

    if(saveState){
         this->model->appendRow(&record);
        //if insert project
        if(id>0){
            //add public group lists
            GroupStorage groupStorage;
            QList<GroupRecord> lists = groupStorage.list(0);
            qDebug()<<"size:"<<lists.size();
            QList<GroupRecord>::iterator iter = lists.begin();
            while(iter!=lists.end()){
                this->model->appendRow(id,&(*iter));
                iter++;
            }
        }

        MessageDialog::info(this,tr("Save successfully"));
    }else{

        MessageDialog::error(this,tr("Save failed ; SQL error:%1").arg(projectStorage.errorText()));
    }

}

void ManagerWindow::onSiteSave()
{
    SiteFrameworkWidget* tab = dynamic_cast<SiteFrameworkWidget*>(ui->siteTabWidget->widget(0));
    if(tab!=nullptr){
        tab->onSave();
    }
}

void ManagerWindow::onSiteTest()
{
    SiteFrameworkWidget* tab = dynamic_cast<SiteFrameworkWidget*>(ui->siteTabWidget->widget(0));
    if(tab!=nullptr){
        tab->onConnect();
    }
}

 void ManagerWindow::onTreeItemClicked(const QModelIndex& index)
 {
    ManagerTreeItem* item = static_cast<ManagerTreeItem*>(index.internalPointer());
    if(item!=nullptr){
        ManagerTreeData::Role role = item->dataRole();
        long long id = item->dataId();
        if(role==ManagerTreeData::PROJECT){
            ProjectStorage projectStorage;
            ProjectRecord record = projectStorage.one(id);
            if(record.id>0l){
                ui->stackedWidget->setCurrentIndex(0);
                ProjectEditWidget* tab = dynamic_cast<ProjectEditWidget*>(ui->projectTabWidget->widget(0));
                tab->setFormData(record);
            }
        }else if(role == ManagerTreeData::GROUP){

        }else if(role == ManagerTreeData::SITE){
            SiteStorage siteProject;
            SiteRecord record = siteProject.one(id);
            if(record.id>0l){
                ui->stackedWidget->setCurrentIndex(1);
                SiteFrameworkWidget* tab = dynamic_cast<SiteFrameworkWidget*>(ui->siteTabWidget->widget(0));
                SiteStorage siteStorage;
                SiteRecord record = siteStorage.one(id);
                if(record.id>0l){
                    tab->setFormData(record);
                }
            }
        }

        this->handle->setActionGroupStatus(static_cast<int>(role));


    }
 }

 void ManagerWindow::onTriggered(QAction* a,bool checked)
 {
    int v = a->data().toInt();
    ManagerTreeItem* item = static_cast<ManagerTreeItem*>(ui->treeView->selectionModel()->currentIndex().internalPointer());
    if(item==nullptr){
        return ;
    }
    //qDebug()<<"point:"<<item->dataId()<<";role:"<<item->dataRole();
    long long id = item->dataId();
    ManagerTreeData::Role role = item->dataRole();

    if(v==SplitterHandleWidget::A_NEW_PROJECT){
        ui->stackedWidget->setCurrentIndex(0);
        ProjectEditWidget* tab = dynamic_cast<ProjectEditWidget*>(ui->projectTabWidget->widget(0));
        tab->clearFormData();
    }else if(v==SplitterHandleWidget::A_DEL_PROJECT){
        if(MessageDialog::confirm(this,tr("Are you want to delete selected project and sites?"),QMessageBox::Ok|QMessageBox::Cancel)==QMessageBox::Ok){
            ProjectStorage db;
            db.del(id);
            SiteStorage siteStorage;
            siteStorage.del_list(id);
            this->initData();//refresh tree list
        }
    }else if(v==SplitterHandleWidget::A_NEW_SITE){
        if(role==ManagerTreeData::PROJECT){

        }else if(role==ManagerTreeData::GROUP){
            id = item->parentItem()->dataId();
        }else{
            return;
        }
        ui->stackedWidget->setCurrentIndex(1);
        SiteFrameworkWidget* tab = dynamic_cast<SiteFrameworkWidget*>(ui->siteTabWidget->widget(0));
        tab->clearFormData();
        tab->setFormData(SiteRecord());
        tab->setPid(id);
        //tab->onTypeChanged(-1);
    }else if(v==SplitterHandleWidget::A_DEL_SITE){
        if(MessageDialog::confirm(this,tr("Are you want to delete selected site?"),QMessageBox::Ok|QMessageBox::Cancel)==QMessageBox::Ok){
            SiteStorage db;
            SiteRecord r = db.one(id);
            model->removeRow(&r);
            db.del(id);
        }
    }else if(v==SplitterHandleWidget::A_MOVE_UP){



    }else if(v==SplitterHandleWidget::A_MOVE_DOWN){

    }

 }


}
