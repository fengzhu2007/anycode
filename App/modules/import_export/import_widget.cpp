#include "import_widget.h"
#include "ui_import_widget.h"
#include "import_select_widget.h"
#include "import_export_dialog.h"
#include "select_tree_model.h"
#include "storage/project_storage.h"
#include "storage/site_storage.h"
#include "backup_restore.h"
#include "components/message_dialog.h"
#include <w_toast.h>
#include <QFileDialog>
#include <QStackedWidget>



namespace ady{
ImportWidget::ImportWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ImportWidget)
{
    ui->setupUi(this);

    connect(ui->browser,&QPushButton::clicked,this,&ImportWidget::onBrowser);
    connect(ui->selectAll,&QCheckBox::stateChanged,this,&ImportWidget::onStateChanged);
    connect(ui->expandAll,&QCheckBox::stateChanged,this,&ImportWidget::onStateChanged);
    m_model = new SelectTreeModel(ui->treeView);
    ui->treeView->setModel(m_model);
    this->initView();
}

ImportWidget::~ImportWidget()
{
    delete ui;
}

void ImportWidget::initView(){

}

bool ImportWidget::ok(){
    if(m_path.isEmpty()){
        return false;
    }
    auto stacked = static_cast<QStackedWidget*>(parentWidget());
    auto widget = static_cast<ImportSelectWidget*>(stacked->widget(ImportExportDialog::Import_Select));
    int ret = widget->result();
    bool merge = false;
    if(ret==0 || ret==2){
        //backup and overwrite
        if(ret==0){
            //backup
            const QString directory = widget->directory();
            const QString filename = widget->filename();

            QFile file(directory+"/"+filename);
            if(file.exists()){
                if(MessageDialog::confirm(this,tr("File Exists"),tr("The file already exists. Do you want to overwrite it?"))!=QMessageBox::Yes){
                    return false;
                }
            }
            BackupRestore br;
            auto ret = br.saveAll(directory,filename);
            if(ret!=BackupRestore::None){
                if(ret==BackupRestore::Invalid_Filename){
                    MessageDialog::info(this,tr("Invalid filename."));
                }else if(ret==BackupRestore::Invalid_Directory){
                    MessageDialog::info(this,tr("Invalid directory."));
                }else if(ret==BackupRestore::Write_Failed){
                    MessageDialog::info(this,tr("File write failed."));
                }
                return false;
            }
        }
        ProjectStorage().delAll();
        SiteStorage().delAll();
    }else if(ret==1){
        //merge
        merge = true;
    }
    BackupRestore br;
    br.readFile(m_path);
    auto list = m_model->m_root->checkedList();
    for(auto one:list){
        QList<long long>sites;
        auto sitelist = one->checkedList();
        for(auto s:sitelist){
            sites<<s->m_id;
        }
        br.restore(one->m_id,sites,merge);
    }
    return true;
}

void ImportWidget::onBrowser(){
    QString file = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("All Files (*.*)"));
    if(!file.isEmpty()){
        const QString errorMsg = m_model->readFile(file);
        if(!errorMsg.isEmpty()){
            wToast::showText(errorMsg);
        }else{
            m_path = file;
        }
    }
}

void ImportWidget::onStateChanged(int state){
    auto sender = this->sender();
    if(sender==ui->selectAll){
        auto model = static_cast<SelectTreeModel*>(ui->treeView->model());
        model->checkedAll(state==Qt::Checked);
    }else if(sender==ui->expandAll){
        state==Qt::Checked?ui->treeView->expandAll():ui->treeView->collapseAll();
    }
}

}
