#include "AboutDialog.h"
#include "ui_aboutdialog.h"
#include "common.h"
namespace ady {
    AboutDialog::AboutDialog(QWidget* parent)
        :QDialog(parent), ui(new Ui::AboutDialog){
        ui->setupUi(this);
        ui->appName->setText(APP_NAME + " " + APP_VERSION);
        connect(ui->closeButton,&QPushButton::clicked,this,&QDialog::close);
    }


    AboutDialog::~AboutDialog()
    {
        delete ui;
    }

}
