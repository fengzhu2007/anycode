#include "import_export_dialog.h"
#include "ui_import_export_dialog.h"
#include "select_widget.h"
#include "export_widget.h"
#include "export_to_widget.h"
#include "import_select_widget.h"
#include "import_widget.h"
#include "core/des.h"
#include <w_toast.h>

namespace ady{
ImportExportDialog* ImportExportDialog::instance = nullptr;


const QString key = "Anycode00";
const QString vt = "Anycode11";


class ImportExportDialogPrivate{
public:
    QList<QWidget*> widgets;

};

ImportExportDialog::ImportExportDialog(QWidget *parent)
    : wDialog(parent)
    , ui(new Ui::ImportExportDialog)
{

    this->setStyleSheet(QString::fromUtf8("#footer,#header{background:#f5f5f5}"));
    //this->setStyleSheet(".wDialog{background:red}");
    ui->setupUi(this);

    d = new ImportExportDialogPrivate;
    this->resetupUi();

    ui->icon->setPixmap(QIcon(":/Resource/icons/LibrarySettings_16x.svg").pixmap({48,48}));

    connect(ui->next,&QPushButton::clicked,this,&ImportExportDialog::onClicked);
    connect(ui->previous,&QPushButton::clicked,this,&ImportExportDialog::onClicked);
    connect(ui->ok,&QPushButton::clicked,this,&ImportExportDialog::onClicked);
    connect(ui->cancel,&QPushButton::clicked,this,&ImportExportDialog::close);

    connect(ui->stackedWidget,&QStackedWidget::currentChanged,this,&ImportExportDialog::onCurrentChanged);

    this->initView();


}

ImportExportDialog::~ImportExportDialog()
{
    delete d;
    delete ui;
}


ImportExportDialog* ImportExportDialog::getInstance(){
    return instance;
}


ImportExportDialog* ImportExportDialog::open(QWidget* parent){
    if(instance==nullptr){
        instance = new ImportExportDialog(parent);
        instance->setModal(true);
    }
    instance->show();
    return instance;
}

QString ImportExportDialog::encode(const QString& str){
    return DES::encode(str,key,vt);
}

QString ImportExportDialog::decode(const QString& str){
    return DES::decode(str,key,vt);
}

void ImportExportDialog::initView(){
    d->widgets<<new SelectWidget(ui->stackedWidget);
    d->widgets<<new ExportWidget(ui->stackedWidget);
    d->widgets<<new ExportToWidget(ui->stackedWidget);
    d->widgets<<new ImportSelectWidget(ui->stackedWidget);
    d->widgets<<new ImportWidget(ui->stackedWidget);
    for(auto widget:d->widgets){
        ui->stackedWidget->addWidget(widget);
    }
}

void ImportExportDialog::onClicked(){
    auto sender = this->sender();
    if(sender==ui->next){
        int current = ui->stackedWidget->currentIndex();
        if(current==0){
            auto widget = static_cast<SelectWidget*>(d->widgets.at(current));
            int option = widget->current();
            if(option>-1){
                ui->stackedWidget->setCurrentIndex(option);
            }
        }else if(current==Export){
            auto list = static_cast<ExportWidget*>(d->widgets.at(current))->result();
            if(list.size()>0){
                ui->stackedWidget->setCurrentIndex(Export_To);
            }
        }else if(current==Import_Select){
            auto ret = static_cast<ImportSelectWidget*>(d->widgets.at(current))->result();
            if(ret>-1){
                ui->stackedWidget->setCurrentIndex(Import);
            }
        }
    }else if(sender==ui->previous){
        int current = ui->stackedWidget->currentIndex();
        if(current==0){
            return;
        }
        if(current==Export||current==Import_Select){
            ui->stackedWidget->setCurrentIndex(0);
        }else{
            ui->stackedWidget->setCurrentIndex(current-1);
        }
    }else if(sender==ui->ok){
        int current = ui->stackedWidget->currentIndex();
        if(current==Export_To){
            bool ret = static_cast<ExportToWidget*>(d->widgets.at(current))->ok();
            if(ret){
                wToast::showText(tr("Export succeeded"));
                this->close();
            }
        }else if(current==Import){
            bool ret = static_cast<ImportWidget*>(d->widgets.at(current))->ok();
            if(ret){
                wToast::showText(tr("Import succeeded"));
                this->close();
            }
        }
    }
}

void ImportExportDialog::onCurrentChanged(int i){
    if(i==0){
        ui->previous->setEnabled(false);
    }else{
        ui->previous->setEnabled(true);
    }

    if(i==Export_To||i==Import){
        ui->ok->setEnabled(true);
        ui->next->setEnabled(false);
        ui->ok->setFocus();
    }else{
        ui->ok->setEnabled(false);
        ui->next->setEnabled(true);
    }


}

}
