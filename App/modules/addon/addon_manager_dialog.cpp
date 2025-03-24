#include "addon_manager_dialog.h"
#include "ui_addon_manager_dialog.h"
#include "addon_manager_model.h"
#include "components/listview/listview_model.h"
namespace ady{
AddonManagerDialog* AddonManagerDialog::instance = nullptr;


class AddonManagerDialogPrivate{
public:
    AddonManagerModel* model;
};

AddonManagerDialog::AddonManagerDialog(QWidget *parent)
    : wDialog(parent)
    , ui(new Ui::AddonManagerDialog)
{

    this->setStyleSheet(".ady--ListView{border:0;}");
    ui->setupUi(this);

    d = new AddonManagerDialogPrivate;
    d->model = new AddonManagerModel(ui->listView);
    connect(ui->close,&QPushButton::clicked,this,&AddonManagerDialog::close);
    connect(ui->listView,&ListView::itemClicked,this,&AddonManagerDialog::onItemClicked);
    this->resetupUi();
    ui->listView->setModel(d->model);
    this->initView();

    //ui->widget->start();
}

void AddonManagerDialog::initView(){

    QList<AddonItem> list;
    list << AddonItem{true,"FTP","The File Transfer Protocol (FTP) is a standard communication protocol used for the transfer of computer files from a server to a client on a computer network.","Official","","1.0"};
    list << AddonItem{true,"SFTP","In computing, the SSH File Transfer Protocol (also known as Secure File Transfer Protocol or SFTP) is a network protocol that provides file access, file transfer, and file management over any reliable data stream.","Official","","1.0"};
    list << AddonItem{true,"OSS","AliCloud Object Storage OSS (Object Storage Service) is a massive, secure, low-cost, and highly reliable cloud storage service that provides up to 99.995% service availability. A variety of storage types are available to comprehensively optimize storage costs.","Official","","1.0"};
    list << AddonItem{true,"COS","Cloud Object Storage (COS) is a distributed storage service launched by Tencent Cloud that has no directory hierarchy, no data format restrictions, can accommodate massive data, and supports HTTP/HTTPS protocol access.","Official","","1.0"};
    d->model->setDataSource(list);
}

AddonManagerDialog::~AddonManagerDialog()
{
    delete d;
    delete ui;
}

void AddonManagerDialog::onItemClicked(int index){
    auto item = d->model->itemAt(index);
    //ui->stackedWidget->show();
    ui->stackedWidget->setCurrentIndex(item.installed?0:1);
    ui->author->setText(tr("<strong>Author:</strong>%1").arg(item.author));
    ui->version->setText(tr("<strong>Version:</strong>%1").arg(item.version));
    ui->url->setText(tr("<strong>Home Page:</strong>%1").arg(item.url));
}

AddonManagerDialog* AddonManagerDialog::open(QWidget* parent){
    if(instance==nullptr){
        instance = new AddonManagerDialog(parent);
        instance->setModal(true);
    }
    instance->show();
    return instance;
}
}
