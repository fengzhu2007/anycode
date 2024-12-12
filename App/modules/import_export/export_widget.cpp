#include "export_widget.h"
#include "ui_export_widget.h"
#include "select_tree_model.h"
#include "storage/project_storage.h"
#include "storage/site_storage.h"

namespace ady{
ExportWidget::ExportWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ExportWidget)
{
    ui->setupUi(this);

    //ui->treeView->setModel()
    this->initView();

    ui->expandAll->setCheckState(Qt::Checked);
    connect(ui->selectAll,&QCheckBox::stateChanged,this,&ExportWidget::onStateChanged);
    connect(ui->expandAll,&QCheckBox::stateChanged,this,&ExportWidget::onStateChanged);
}

ExportWidget::~ExportWidget()
{
    delete ui;
}

void ExportWidget::initView(){
    auto model = new SelectTreeModel(ui->treeView);
    auto projectlist = ProjectStorage().all();
    auto sitelist = SiteStorage().all();
    for(auto project:projectlist){
        auto proj = new SelectTreeModelItem(SelectTreeModelItem::Project,project.id,project.name,model->m_root);
        for(auto site:sitelist){
            if(project.id==site.pid){
                auto child = new SelectTreeModelItem(SelectTreeModelItem::Site,site.id,site.name,proj);
                proj->m_children<<child;
            }
        }
        model->m_root->m_children<<proj;
    }
    for(auto site:sitelist){
        if(site.pid==0){
            auto child = new SelectTreeModelItem(SelectTreeModelItem::Site,site.id,site.name,model->m_root);
            model->m_root->m_children<<child;
        }
    }
    //model->m_root->dump();
    ui->treeView->setModel(model);
    ui->treeView->expandAll();
}

QList<SelectTreeModelItem*> ExportWidget::result(){
    return static_cast<SelectTreeModel*>(ui->treeView->model())->m_root->checkedList();
}

void ExportWidget::onStateChanged(int state){
    auto sender = this->sender();
    if(sender==ui->selectAll){
        auto model = static_cast<SelectTreeModel*>(ui->treeView->model());
        model->checkedAll(state==Qt::Checked);
    }else if(sender==ui->expandAll){
        state==Qt::Checked?ui->treeView->expandAll():ui->treeView->collapseAll();
    }
}

}
