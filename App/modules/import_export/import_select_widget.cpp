#include "import_select_widget.h"
#include "ui_import_select_widget.h"
#include "common.h"
#include <QButtonGroup>
#include <QDateTime>
#include <QStandardPaths>
#include <QFileDialog>
namespace ady{
ImportSelectWidget::ImportSelectWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ImportSelectWidget)
{
    ui->setupUi(this);

    auto group = new QButtonGroup(this);
    group->addButton(ui->backupOpt);
    group->addButton(ui->mergeOpt);
    group->addButton(ui->overwriteOpt);

    connect(ui->browser,&QPushButton::clicked,this,&ImportSelectWidget::onBrowser);

    this->initView();
}

ImportSelectWidget::~ImportSelectWidget()
{
    delete ui;
}


void ImportSelectWidget::initView(){
    ui->filename->setText(APP_NAME+"-export-"+QDateTime::currentDateTime().toString("yyyyMMdd")+".json");
    ui->directory->setText(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
}


void ImportSelectWidget::onBrowser(){
    const QString local = QFileDialog::getExistingDirectory(this, tr("Select Export To Directory"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(!local.isEmpty()){
        ui->directory->setText(local);
    }
}


int ImportSelectWidget::result(){
    if(ui->backupOpt->isChecked()){
        return 0;
    }else if(ui->mergeOpt->isChecked()){
        return 1;
    }else if(ui->overwriteOpt->isChecked()){
        return 2;
    }
    return -1;
}
}
