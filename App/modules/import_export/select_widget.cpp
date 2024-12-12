#include "select_widget.h"
#include "ui_select_widget.h"
#include "import_export_dialog.h"
#include <QButtonGroup>
#include <QIcon>
namespace ady{
SelectWidget::SelectWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SelectWidget)
{
    ui->setupUi(this);

    auto group = new QButtonGroup(this);

    group->addButton(ui->exportOpt);
    group->addButton(ui->importOpt);
}

SelectWidget::~SelectWidget()
{
    delete ui;
}


int SelectWidget::current(){
    if(ui->exportOpt->isChecked()){
        return ImportExportDialog::Export;
    }else if(ui->importOpt->isChecked()){
        return ImportExportDialog::Import_Select;
    }else{
        return -1;
    }
}

int SelectWidget::result(){
    return current();
}

}
