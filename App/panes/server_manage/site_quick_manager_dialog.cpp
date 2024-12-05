#include "site_quick_manager_dialog.h"
#include "ui_site_quick_manager_dialog.h"
#include "storage/site_storage.h"
#include "storage/addon_storage.h"
#include "interface/form_panel.h"
#include "components/message_dialog.h"
#include "project/site_type_list_model.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/type.h"
#include "components/list_item_delegate.h"
#include "addon_loader.h"
#include <w_toast.h>

namespace ady{

SiteQuickManagerDialog* SiteQuickManagerDialog::instance = nullptr;


class SiteQuickManagerDialogPrivate{
public:
    QList<AddonRecord> addons;
    QList<SiteRecord>list;
    SiteRecord current;
    QList<FormPanel*> panels;
};

SiteQuickManagerDialog::SiteQuickManagerDialog(QWidget *parent)
    : wDialog(parent)
    , ui(new Ui::SiteQuickManagerDialog)
{
    this->setStyleSheet(".QStackedWidget .QLineEdit,QStackedWidget .QSpinBox{border:1px solid #ccc;height:32px;padding:0 4px 0 4px;font-size:14px}"
                        ".QStackedWidget .QLineEdit:hover,QStackedWidget .QSpinBox:hover{border-color:#007acc}"
                        ".QStackedWidget .QLineEdit:focus,QStackedWidget .QSpinBox:focus{border-color:#007acc}"
                        ".QStackedWidget .QPushButton#del,QStackedWidget .QComboBox{height:28px}"
                        ".QStackedWidget .QPushButton#next,QStackedWidget .QPushButton#connect,QStackedWidget .QPushButton#saveSite,QStackedWidget .QPushButton#newSite{height:22px}"
                        ".QTabWidget::pane{border-top:1px solid #ccc;margin-top:-1px}"
                        ".QTabBar::tab{height:28px}");

    ui->setupUi(this);
    this->resetupUi();

    d = new SiteQuickManagerDialogPrivate;

    connect(ui->newSite,&QPushButton::clicked,this,&SiteQuickManagerDialog::onNewSite);
    connect(ui->connect,&QPushButton::clicked,this,&SiteQuickManagerDialog::onConnect);
    connect(ui->del,&QPushButton::clicked,this,&SiteQuickManagerDialog::onDel);
    connect(ui->list,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&SiteQuickManagerDialog::onSiteChanged);
    connect(ui->type,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&SiteQuickManagerDialog::onTypeChanged);
    connect(ui->tabWidget,&QTabWidget::currentChanged, this, &SiteQuickManagerDialog::onAdjustHeight);
    connect(ui->saveSite,&QPushButton::clicked,this,&SiteQuickManagerDialog::onSave);

    QList<AddonRecord> addons;
    AddonStorage addonStorage;
    d->addons = addonStorage.list(1);

    auto model = new SiteTypeListModel(ui->type);
    model->setData(d->addons);
    ui->type->setModel(model);

    ui->list->setItemDelegate(new ListItemDelegate(24,ui->list));
    ui->type->setItemDelegate(new ListItemDelegate(24,ui->type));

    this->initData();
}

SiteQuickManagerDialog::~SiteQuickManagerDialog()
{
    delete d;
    delete ui;
}


SiteQuickManagerDialog* SiteQuickManagerDialog::open(QWidget *parent){
    if(instance==nullptr){
        instance = new SiteQuickManagerDialog(parent);
        instance->setModal(true);
    }
    instance->show();
    return instance;
}


void SiteQuickManagerDialog::initData(){
    //id = 1l;
    //search site list;
    this->initSiteList();
    //this->onNewSite();//clear form item data
}

void SiteQuickManagerDialog::onNewSite(){
    ui->list->setCurrentIndex(-1);
    ui->name->clear();
    ui->type->setCurrentIndex(-1);
    d->current = {};
    ui->type->setEnabled(true);
}


void SiteQuickManagerDialog::onSiteChanged(int i){
    if(i<0){
        return ;
    }
    d->current = d->list.at(i);
    ui->name->setText(d->current.name);
    for(int i=0;i<d->addons.length();i++){
        if(d->addons[i].name==d->current.type){
            ui->type->setCurrentIndex(i);
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
        //set type disable
        ui->type->setEnabled(false);
    }
}


void SiteQuickManagerDialog::onTypeChanged(int i){
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

void SiteQuickManagerDialog::onAdjustHeight(int i){
    if (i < 0 || i >= ui->tabWidget->count()) return;

    QWidget *currentTab = ui->tabWidget->widget(i);
    if (!currentTab) return;

    QSize size = currentTab->sizeHint();
    ui->tabWidget->setMaximumHeight(size.height() + ui->tabWidget->tabBar()->height());
}

void SiteQuickManagerDialog::onDeleteSite(int i){

    auto one = d->list.at(i);
    if(MessageDialog::confirm(this,tr("Are you want to delete selected site[%1]?").arg(one.name),QMessageBox::Ok|QMessageBox::Cancel)==QMessageBox::Ok){
        SiteStorage db;
        db.del(one.id);

        Publisher::getInstance()->post(Type::M_SITE_DELETED,&d->current);//DELETE
        if(one.id==d->current.id){
            //clear data
            this->onNewSite();
        }
    }
}

void SiteQuickManagerDialog::onSave(){
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

        d->current.name = name;
        d->current.type = ((SiteTypeListModel*)ui->type->model())->dataIndex(typeIndex).name;
        d->current.groupid = 3;
        d->current.status = 1;
        SiteStorage siteStorage;

        if(d->current.id>0){
            //update
            siteStorage.update(d->current);
            Publisher::getInstance()->post(Type::M_SITE_UPDATED,&d->current);

            //wToast::showText(tr("Save successful"));
            this->initSiteList();
        }else{
            //insert
            d->current.pid = 0;
            d->current.datetime = QDateTime::currentDateTime().toSecsSinceEpoch();
            long long ret = siteStorage.insert(d->current);
            if(ret>0){
                //append ok
                d->current.id = ret;
                //send message
                Publisher::getInstance()->post(Type::M_SITE_ADDED,&d->current);
                //wToast::showText(tr("Save successful"));
                this->initSiteList();
            }else{
                //failed
                wToast::showText(tr("Save failed"));
            }
        }
        this->close();
    }
}

void SiteQuickManagerDialog::onDel(){
    int index = ui->list->currentIndex();
    if(index>-1 && index < d->list.length()){
        auto item = d->list.at(index);
        if(MessageDialog::confirm(this,tr("Are you want to delete selected site[%1]?").arg(item.name),QMessageBox::Ok|QMessageBox::Cancel)==QMessageBox::Ok){
            Publisher::getInstance()->post(Type::M_SITE_DELETED,&d->current);//close site
            SiteStorage().del(item.id);
            d->list.removeAt(index);
            this->onNewSite();
        }
    }
}

void SiteQuickManagerDialog::onConnect(){

}

void SiteQuickManagerDialog::initSiteList(){
    ui->list->clear();
    SiteStorage siteStorage;
    d->list = siteStorage.list(0l);
    for(auto one:d->list){
        ui->list->addItem(one.name);
    }
}


}
