#include "about_dialog.h"
#include "ui_about_dialog.h"
#include "common.h"
#include <QDebug>
namespace ady {
AboutDialog* AboutDialog::instance = nullptr;
    AboutDialog::AboutDialog(QWidget* parent)
        :wDialog(parent), ui(new Ui::AboutDialog){
        ui->setupUi(this);
        this->resetupUi();
        ui->appName->setText(APP_NAME + " " + APP_VERSION);
        ui->appBuild->setText(tr("Build:%1").arg(BUILD_TIME));
        ui->qtVersion->setText(tr("Qt:%1,Compiler:%2").arg(QT_VERSION_STR).arg(COMPILER_INFO));
        ui->appUrl->setText(tr("Home Page:<a href=\"%1\">%2</a>").arg(APP_URL).arg(APP_URL));
        ui->appUrl->setOpenExternalLinks(true);


        connect(ui->closeButton,&QPushButton::clicked,this,&QDialog::close);
    }

    AboutDialog::~AboutDialog()
    {
        delete ui;
        instance = nullptr;
    }

    AboutDialog* AboutDialog::open(QWidget* parent){
        if(instance==nullptr){
            instance = new AboutDialog(parent);
        }
        instance->show();
        return instance;
    }

}
