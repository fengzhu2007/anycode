#include "AddonManageWindow.h"
#include "ui_AddonManageWindow.h"
#include "AddonModel.h"
namespace ady{
    AddonManageWindow* AddonManageWindow::instance = nullptr;



    AddonManageWindow::AddonManageWindow(QWidget* parent)
        :QDialog(parent),ui(new Ui::AddonManageWindow()){
        ui->setupUi(this);
        AddonStorage storage;
        ;
        AddonModel* model = new AddonModel(ui->treeView);
        model->setList(storage.all());
        ui->treeView->setModel(model);

        ui->treeView->setColumnWidth(AddonModel::File,180);
    }

    AddonManageWindow::~AddonManageWindow()
    {
        delete ui;
        ui = nullptr;
        AddonManageWindow::instance = nullptr;
    }


    AddonManageWindow* AddonManageWindow::getInstance(QWidget*parent)
    {
        if(instance==nullptr){
            instance = new AddonManageWindow(parent);
        }
        return instance;
    }
}
