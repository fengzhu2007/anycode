#include "import_widget.h"
#include "ui_import_widget.h"
#include "import_select_widget.h"
#include "import_export_dialog.h"
#include "select_tree_model.h"
#include "storage/project_storage.h"
#include "storage/site_storage.h"
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
    m_model = new SelectTreeModel(ui->treeView);
    ui->treeView->setModel(m_model);
}

ImportWidget::~ImportWidget()
{
    delete ui;
}

void ImportWidget::initView(){

}

bool ImportWidget::ok(){
    auto stacked = static_cast<QStackedWidget*>(parentWidget());
    auto widget = static_cast<ImportSelectWidget*>(stacked->widget(ImportExportDialog::Import_Select));
    int ret = widget->result();


    return false;
}

void ImportWidget::onBrowser(){
    QString file = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("All Files (*.*)"));
    if(file.isEmpty()){

        const QString errorMsg = m_model->readFile(file);
        if(!errorMsg.isEmpty()){
            wToast::showText(errorMsg);
        }
    }
}

}
