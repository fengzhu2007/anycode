#include "export_to_widget.h"
#include "ui_export_to_widget.h"
#include "import_export_dialog.h"
#include "export_widget.h"
#include "select_tree_model.h"
#include "components/message_dialog.h"
#include "backup_restore.h"
#include <QStandardPaths>
#include <QDateTime>
#include <QFileDialog>
#include <QStackedWidget>
#include <QJsonArray>

#include <QDebug>

namespace ady{
ExportToWidget::ExportToWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ExportToWidget)
{
    ui->setupUi(this);
    connect(ui->browser,&QPushButton::clicked,this,&ExportToWidget::onBrowser);
    this->initView();
}

ExportToWidget::~ExportToWidget()
{
    delete ui;
}


void ExportToWidget::initView(){
    ui->filename->setText(BackupRestore::defaultFilename());
    ui->directory->setText(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
}

void ExportToWidget::onBrowser(){
    const QString local = QFileDialog::getExistingDirectory(this, tr("Select Export To Directory"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(!local.isEmpty()){
        ui->directory->setText(local);
    }
}

bool ExportToWidget::ok(){
    auto stacked = static_cast<QStackedWidget*>(parentWidget());
    auto widget = static_cast<ExportWidget*>(stacked->widget(ImportExportDialog::Export));
    auto list = widget->result();


    BackupRestore br;

    QJsonArray array;
    for(auto one:list){
        if(one->m_type==SelectTreeModelItem::Project){
            QList<long long> ids;
            auto sites = one->checkedList();
            for(auto s:sites){
                ids.push_back(s->m_id);
            }
            br.appendProject(one->m_id,ids);
        }else if(one->m_type==SelectTreeModelItem::Site){
            br.appendSite(one->m_id);
        }
    }

    QString directory = ui->directory->text();
    QString filename = ui->filename->text();


    QFile file(directory+"/"+filename);
    if(file.exists()){
        if(MessageDialog::confirm(this,tr("File Exists"),tr("The file already exists. Do you want to overwrite it?"))!=QMessageBox::Yes){
            return false;
        }
    }
    auto ret = br.save(directory,filename);
    if(ret==BackupRestore::None){
        return true;
    }else{
        if(ret==BackupRestore::Invalid_Filename){
            MessageDialog::info(this,tr("Invalid filename."));
        }else if(ret==BackupRestore::Invalid_Directory){
            MessageDialog::info(this,tr("Invalid directory."));
        }else if(ret==BackupRestore::Write_Failed){
            MessageDialog::info(this,tr("File write failed."));
        }
    }
    return false;
}

}
