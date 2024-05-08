#include "DonationDialog.h"
#include "ui_donationdialog.h"
#include "common.h"
namespace ady {
    DonationDialog::DonationDialog(QWidget* parent)
        :QDialog(parent), ui(new Ui::DonationDialog){
        ui->setupUi(this);
    }


    DonationDialog::~DonationDialog()
    {
        delete ui;
    }

}
