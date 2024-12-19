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
    connect(group,QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),this,&ImportSelectWidget::onToggle);

    this->initView();
}

ImportSelectWidget::~ImportSelectWidget()
{
    delete ui;
}


void ImportSelectWidget::initView(){
    ui->filename->setText(APP_NAME+"-export-"+QDateTime::currentDateTime().toString("yyyyMMdd")+".json");
    ui->directory->setText(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    this->onToggle(nullptr);
}


void ImportSelectWidget::onBrowser(){
    const QString local = QFileDialog::getExistingDirectory(this, tr("Select Export To Directory"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(!local.isEmpty()){
        ui->directory->setText(local);
    }
}

void ImportSelectWidget::onToggle(QAbstractButton *button){
    bool ret = ui->backupOpt->isChecked();
    ui->filename->setEnabled(ret);
    ui->directory->setEnabled(ret);
    ui->browser->setEnabled(ret);
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

QString ImportSelectWidget::filename(){
    return ui->filename->text();
}

QString ImportSelectWidget::directory(){
    return ui->directory->text();
}

}
