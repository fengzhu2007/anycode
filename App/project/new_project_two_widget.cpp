#include "new_project_two_widget.h"
#include "ui_new_project_two_widget.h"
#include "new_project_window.h"
#include "storage/site_storage.h"
#include "storage/addon_storage.h"
#include "storage/group_storage.h"
#include "site_list_model.h"
#include "site_group_list_model.h"
#include "site_type_list_model.h"
#include "addon_loader.h"
#include "interface/form_panel.h"
#include "components/message_dialog.h"
#include "w_toast.h"
#include <QStyleOption>
#include <QPainter>
#include <QPushButton>
#include <QMessageBox>
#include <QList>
#include <QMenu>
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
    this->setStyleSheet(QString::fromUtf8("QTabWidget::pane{border:0;border-top:1px solid #ccc}"
                                          "QAbstractScrollArea>QWidget{background:transparent}"));
    ui->setupUi(this);
    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);

    NewProjectWindow* window = (NewProjectWindow*)this->parentWidget()->parentWidget();
    connect(ui->newSite,&QPushButton::clicked,this,&NewProjectTwoWidget::onNewSite);
    connect(ui->previous,&QPushButton::clicked,window,&NewProjectWindow::previous);
    connect(ui->listView,&ListView::itemClicked,this,&NewProjectTwoWidget::onSiteItemClicked);
    connect(ui->type,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&NewProjectTwoWidget::onTypeChanged);
    connect(ui->tabWidget,&QTabWidget::currentChanged, this, &NewProjectTwoWidget::onAdjustHeight);
    connect(ui->save,&QPushButton::clicked,this,&NewProjectTwoWidget::onSave);
    connect(ui->listView, &QWidget::customContextMenuRequested, this, &NewProjectTwoWidget::showSiteContextMenu);

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
    this->onNewSite();//clear form item data
}

void NewProjectTwoWidget::onNewSite(){
    ui->name->clear();
    ui->type->setCurrentIndex(-1);
    ui->group->setCurrentIndex(-1);
    ui->status->setChecked(false);
    ui->listView->clearSelected();
    d->current = {};
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
        ui->tabWidget->show();
        AddonRecord one = d->addons.at(i);
        QString name = one.name;
        AddonLoader* loader = AddonLoader::getInstance();
        bool ret = loader->loadFile(one.file);
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
    }else{
        ui->tabWidget->hide();
    }
}

void NewProjectTwoWidget::onAdjustHeight(int i){
    if (i < 0 || i >= ui->tabWidget->count()) return;

    QWidget *currentTab = ui->tabWidget->widget(i);
    if (!currentTab) return;

    QSize size = currentTab->sizeHint();
    ui->tabWidget->setMaximumHeight(size.height() + ui->tabWidget->tabBar()->height());
}

void NewProjectTwoWidget::onDeleteSite(int i){
    auto model = static_cast< SiteListModel*>(ui->listView->model());
    if(model!=nullptr){
        SiteRecord one = model->itemAt(i);
        if(MessageDialog::confirm(this,tr("Are you want to delete selected site[%1]?").arg(one.name),QMessageBox::Ok|QMessageBox::Cancel)==QMessageBox::Ok){
            SiteStorage db;
            db.del(one.id);
            model->itemRemoved(i);
            if(one.id==d->current.id){
                //clear data
                this->onNewSite();
            }
        }
    }
}

void NewProjectTwoWidget::onSave(){
    NewProjectWindow* window = (NewProjectWindow*)parentWidget()->parentWidget();
    long long id = window->currentProjectId();
    if(d->panels.length()==0){
        //Toast
    }else{
        //SiteRecord record;
        //d->current
        foreach(auto one,d->panels){
            if(!one->validateFormData(d->current)){
                return ;
            }
        }

        QString name = ui->name->text();
        if(name.isEmpty()){
            QMessageBox::information(this,tr("Warning"),tr("Name required"));
            return ;
        }
        int typeIndex = ui->type->currentIndex();
        if(typeIndex<0){
            QMessageBox::information(this,tr("Warning"),tr("Type required"));
            return ;
        }
        int groupIndex = ui->group->currentIndex();
        if(groupIndex<0){
            QMessageBox::information(this,tr("Warning"),tr("Group required"));
            return ;
        }

        d->current.name = name;
        d->current.type = ((SiteTypeListModel*)ui->type->model())->dataIndex(typeIndex).name;
        d->current.groupid = ((SiteGroupListModel*)ui->group->model())->dataIndex(groupIndex).id;
        d->current.status = ui->status->isChecked()?1:0;
        SiteStorage siteStorage;
        auto model = static_cast<SiteListModel*>(ui->listView->model());
        if(d->current.id>0){
            //update
            siteStorage.update(d->current);

            int index = model->index(d->current);
            //update item
            if(index>-1)
                model->updateItem(d->current);
                //ui->listView->setSelection(QList<int>{index});

            wToast::showText(tr("Save successful"));
        }else{
            //insert
            d->current.pid = id;
            d->current.datetime = QDateTime::currentDateTime().toSecsSinceEpoch();
            long long ret = siteStorage.insert(d->current);
            if(ret>0){
                //append ok
                d->current.id = ret;
                //append item
                model->appendItem(d->current);
                int index = model->count() - 1;
                ui->listView->setSelection(QList<int>{index});
                wToast::showText(tr("Save successful"));
            }else{
                //failed
                wToast::showText(tr("Save failed"));
            }
        }
    }
}

void NewProjectTwoWidget::showSiteContextMenu(const QPoint &pos){
    int row = ui->listView->findItem(pos);
    if(row>=0){
        QMenu contextMenu(this);
        contextMenu.addAction(tr("Edit"),[this,row](){
            this->onSiteItemClicked(row);
        });
        contextMenu.addAction(tr("Delete"),[this,row](){
            this->onDeleteSite(row);
        });
        contextMenu.exec(QCursor::pos());

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
