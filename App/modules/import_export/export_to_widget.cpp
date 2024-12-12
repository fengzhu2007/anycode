#include "export_to_widget.h"
#include "ui_export_to_widget.h"
#include "import_export_dialog.h"
#include "export_widget.h"
#include "storage/project_storage.h"
#include "storage/site_storage.h"
#include "select_tree_model.h"
#include "components/message_dialog.h"
#include "common/utils.h"
#include "common.h"
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
    ui->filename->setText(APP_NAME+"-export-"+QDateTime::currentDateTime().toString("yyyyMMdd")+".json");
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
    auto projectlist = ProjectStorage().all();
    auto sitelist = SiteStorage().all();
    auto findProject = [projectlist](long long id){
        for(auto one:projectlist){
            if(one.id==id){
                return one;
            }
        }
    };
    auto findSite = [sitelist](long long id){
        for(auto one:sitelist){
            if(one.id==id){
                return one;
            }
        }
    };
    QJsonArray array;
    for(auto one:list){
        if(one->m_type==SelectTreeModelItem::Project){
            auto r = findProject(one->m_id);
            if(r.id>0){
                if(!r.cvs_password.isEmpty()){
                    r.cvs_password = ImportExportDialog::encode(r.cvs_password);//password encode
                }

                auto sites = one->checkedList();
                QJsonArray sitesJson;
                for(auto s:sites){
                    auto rr = findSite(s->m_id);
                    if(rr.id>0){
                        if(!rr.password.isEmpty()){
                            rr.password = ImportExportDialog::encode(rr.password);//password encode
                        }
                        sitesJson<<rr.toJson();
                    }
                }
                auto proj = r.toJson();
                proj.insert("list",sitesJson);
                QJsonObject item = {
                    {"type",one->m_type},
                    {"item",proj}

                };
                array<<item;
            }
        }else if(one->m_type==SelectTreeModelItem::Site){
            auto rr = findSite(one->m_id);
            if(rr.id>0){
                if(!rr.password.isEmpty()){
                    rr.password = ImportExportDialog::encode(rr.password);//password encode
                }
                QJsonObject item = {
                    {"type",one->m_type},
                    {"item",rr.toJson()}
                };
                array<<item;
            }
        }
    }

    QJsonObject data = {
        {"version",1},
        {"name",APP_NAME+" Export Data"},
        {"creator",APP_NAME+" "+APP_VERSION},
        {"date",QDateTime::currentDateTime().toString()},
        {"data",array},
    };

    QString directory = ui->directory->text();
    QString filename = ui->filename->text();
    if(!Utils::isFilename(filename)){
        MessageDialog::error(this,tr("Invalid file name."));
        return false;
    }

    QFile file(directory+"/"+filename);
    if(file.exists()){
        if(MessageDialog::confirm(this,tr("File Exists"),tr("The file already exists. Do you want to overwrite it?"))!=QMessageBox::Ok){
            return false;
        }
    }

    //mkdir
    QDir dir;
    if(QFileInfo::exists(directory)==false && !dir.mkpath(directory)){
        MessageDialog::error(this,tr("Invalid directory."));
        return false;
    }

    QJsonDocument doc;
    doc.setObject(data);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        MessageDialog::error(this,tr("Export failed."));
        return false;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out <<doc.toJson(QJsonDocument::Indented);
    file.close();
    return true;
}

}
