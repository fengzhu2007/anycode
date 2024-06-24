#include "SiteFrameworkWidget.h"
#include "ui_SiteFrameworkWidget.h"
#include "SiteGroupListModel.h"
#include "SiteTypeListModel.h"
#include "../AddonLoader.h"
#include "interface/FormPanel.h"
#include "ManagerWindow.h"
#include "ManagerTreeModel.h"
#include "ManagerTreeItem.h"
#include "components/MessageDialog.h"
#include <QDateTime>

#include <QDebug>
namespace ady {

    SiteFrameworkWidget::SiteFrameworkWidget(QWidget* parent,long long pid)
        :QWidget(parent),
        ui(new Ui::SiteFrameworkWidgetUI)
    {
        ui->setupUi(this);
        this->id = 0l;
        this->pid = pid;

        this->initView();
        this->initData();
    }

    void SiteFrameworkWidget::initView()
    {
        ui->typeComboBox->setModel(new SiteTypeListModel(ui->typeComboBox));
        ui->groupComboBox->setModel(new SiteGroupListModel(ui->groupComboBox));

        //connect(ui->typeComboBox,&QComboBox::currentIndexChanged,this,&SiteFrameworkWidget::onTypeChanged);
        connect(ui->typeComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(onTypeChanged(int)));
    }

    void SiteFrameworkWidget::initData()
    {
        GroupStorage groupStorage;
        QList<GroupRecord>lists1 = groupStorage.list(this->pid);

        AddonStorage addonStorage;
        QList<AddonRecord>lists2 = addonStorage.list(1);
        //qDebug()<<"size:"<<lists2.size();

        SiteGroupListModel* model1 = static_cast<SiteGroupListModel*>(ui->groupComboBox->model());
        SiteTypeListModel* model2 = static_cast<SiteTypeListModel*>(ui->typeComboBox->model());

        model1->setData(lists1);
        model2->setData(lists2);


    }

    void SiteFrameworkWidget::getFormData(SiteRecord& record)
    {
        int index = ui->groupComboBox->currentIndex();
        if(index>=0){
            SiteGroupListModel* model = static_cast<SiteGroupListModel*>(ui->groupComboBox->model());
            GroupRecord r = model->dataIndex(index);
            record.groupid = r.id;
        }

        index = ui->typeComboBox->currentIndex();
        if(index>=0){
            SiteTypeListModel* model = static_cast<SiteTypeListModel*>(ui->typeComboBox->model());
            AddonRecord r = model->dataIndex(index);
            record.type = r.name;
        }

    }

    void SiteFrameworkWidget::setFormData(SiteRecord record)
    {
        this->record = record;
        if(this->record.id>0){
            this->setPid(record.pid);

            ui->nameLineEdit->setText(this->record.name);

            //init
            {
                SiteGroupListModel* model = static_cast<SiteGroupListModel*>(ui->groupComboBox->model());
                int index = model->indexOf(this->record.groupid);
                if(index>=0 && index < model->rowCount()){
                    ui->groupComboBox->setCurrentIndex(index);
                }
            }

            SiteTypeListModel* model = static_cast<SiteTypeListModel*>(ui->typeComboBox->model());

            int index = ui->typeComboBox->currentIndex();
            QString name = "";
            if(index>=0 && index < model->rowCount()){
                AddonRecord r = model->dataIndex(index);
                name = r.name;
            }
            qDebug()<<"name:"<<name;
            qDebug()<<"type:"<<this->record.type;
            if(name!=this->record.type){
                //model.data
                int index = model->indexOf(this->record.type);
                if(index>=0){
                    ui->typeComboBox->setCurrentIndex(index);
                    //onTypeChanged(index);
                }
            }


            for(auto panel:panels){
                panel->initFormData(record);
            }

        }
    }

    void SiteFrameworkWidget::setPid(long long pid)
    {
        this->pid = pid;
        this->initData();
    }

    void SiteFrameworkWidget::clearFormData()
    {
        ui->nameLineEdit->clear();
        ui->typeComboBox->setCurrentIndex(-1);
        ui->groupComboBox->setCurrentIndex(-1);
    }

    bool SiteFrameworkWidget::validateFormData(SiteRecord& record)
    {
        QString name = ui->nameLineEdit->text();
        if(name.isEmpty()){
            QMessageBox::information(this,tr("Warning"),tr("Name required"));
            return false;
        }

        int typeIndex = ui->typeComboBox->currentIndex();
        if(typeIndex<0){
            QMessageBox::information(this,tr("Warning"),tr("Type required"));
            return false;
        }
        int groupIndex = ui->groupComboBox->currentIndex();
        if(groupIndex<0){
            QMessageBox::information(this,tr("Warning"),tr("Group required"));
            return false;
        }

        record.name = name;
        record.type = ((SiteTypeListModel*)ui->typeComboBox->model())->dataIndex(typeIndex).name;
        record.groupid = ((SiteGroupListModel*)ui->groupComboBox->model())->dataIndex(groupIndex).id;
        if(record.pid==0){
            record.pid = pid;
        }

        QList<FormPanel*>::iterator iter = panels.begin();
        while(iter!=panels.end()){
            if((*iter)->validateFormData(record)==false){
                return false;
            }
            iter++;
        }
        return true;
    }


    void SiteFrameworkWidget::onTypeChanged(int index)
    {
        //qDebug()<<"type changed index:"<<index;
        QTabWidget* parent = static_cast<QTabWidget*>(this->parentWidget()->parentWidget());
        if(index==-1){
            QLayoutItem *child = ui->verticalLayout->takeAt(1);
            if(child!=nullptr){
                QWidget * w = child->widget();
                ui->verticalLayout->removeItem(child);
                delete child;
                delete w;
            }
            int count = parent->count();
            for(int i=1;i<count;i++){
                parent->removeTab(1);
            }
            return ;
        }



        SiteTypeListModel* model = static_cast<SiteTypeListModel*>(ui->typeComboBox->model());
        AddonRecord r = model->dataIndex(index);
        QString name = r.name;

        //QLayoutItem *item = ui->verticalLayout->itemAt(1);

        QLayoutItem *child = ui->verticalLayout->takeAt(1);
        if(child!=nullptr){
            QWidget * w = child->widget();
            ui->verticalLayout->removeItem(child);
            delete child;
            delete w;
        }

        int count = parent->count();
        for(int i=1;i<count;i++){
            parent->removeTab(1);
        }

        panels.clear();

        AddonLoader* loader = AddonLoader::getInstance();
        bool ret = loader->load(r.file);
        if(ret){
            int size =loader->getFormPanelSize(name);
            for(int i=0;i<size;i++){
                FormPanel* panel = loader->getFormPanel(i==0 ? ((QWidget*)this):parent,name,i);
                if(panel!=nullptr){
                    panels.push_back(panel);
                    if(i==0){
                        ui->verticalLayout->addWidget((QWidget*)panel);
                    }else{
                        parent->addTab(panel,panel->windowTitle());
                    }
                }
            }
        }

        if(this->record.id>0){
            //init form data
            QList<FormPanel*>::iterator iter = panels.begin();
            while(iter!=panels.end()){
                (*iter)->initFormData(this->record);
                iter++;
            }
        }



    }


    void SiteFrameworkWidget::onSave()
    {
        bool ret = this->validateFormData(this->record);
        if(ret){
            //save data
            SiteStorage siteStorage;

            if(this->record.id>0){
                //update data
                ret = siteStorage.update(this->record);
                if(ret){
                    //update item
                    //QMessageBox::information(this,tr("Information"),tr("Save successful"));
                    MessageDialog::info(this,tr("Save successful"));
                }else{
                    MessageDialog::error(this,tr("Save faild ; SQL error:%1").arg(siteStorage.errorText()));
                    return ;
                }
            }else{
                //insert data
                this->record.datetime = QDateTime::currentDateTime().toSecsSinceEpoch();
                long long id = siteStorage.insert(this->record);
                if(id>0l){
                    this->record.id = id;
                    //insert item
                    MessageDialog::info(this,tr("Save successful"));
                }else{
                    MessageDialog::error(this,tr("Save faild ; SQL error:%1").arg(siteStorage.errorText()));
                    return ;
                }
            }
            ManagerWindow* win = static_cast<ManagerWindow*>(this->topLevelWidget());
            ManagerTreeModel* model = win->getTreeModel();
            if(model!=nullptr){
                //model->notifyRecordChanged(&this->record);
                model->appendRow(&this->record);
            }
        }




    }

    void SiteFrameworkWidget::onConnect()
    {
        bool ret = this->validateFormData(this->record);
        if(ret){
            AddonLoader* loader = AddonLoader::getInstance();

            SiteTypeListModel* model = static_cast<SiteTypeListModel*>(ui->typeComboBox->model());
            AddonRecord r = model->dataIndex(ui->typeComboBox->currentIndex());
            bool ret = loader->load(r.file);
            if(ret){
                int result = loader->requestConnect(&this->record);
                if(result==0){
                    MessageDialog::info(topLevelWidget(),tr("Connect succeful"));
                    return ;
                }else if(result==1){
                    MessageDialog::info(topLevelWidget(),tr("Remote dir is not exists"));
                    return ;
                }else if(result==2){
                    MessageDialog::info(topLevelWidget(),tr("Connect failed"));
                    return ;
                }else{
                    MessageDialog::info(topLevelWidget(),tr("Unknown exception"));
                    return;
                }
            }
        }
    }
}
