#include "new_folder_dialog.h"
#include "ui_new_folder_dialog.h"
#include "common/utils.h"
#include "w_toast.h"
#include <QDebug>
namespace ady{
class NewFolderDialogPrivate{
public:
    long long id;
    QString path;
};

NewFolderDialog::NewFolderDialog(long long id,const QString& path,QWidget* parent)
    :wDialog(parent)
{
    ui = new Ui::NewFolderDialog;
    d = new NewFolderDialogPrivate;
    d->id = id;
    d->path = path;

    ui->setupUi(this);
    connect(ui->ok,&QPushButton::clicked,this,&NewFolderDialog::onOk);
    connect(ui->cancel,&QPushButton::clicked,this,&NewFolderDialog::close);
    this->resetupUi();
}

NewFolderDialog::~NewFolderDialog(){
    delete d;
    delete ui;
}

NewFolderDialog* NewFolderDialog::open(long long id,const QString& path,QWidget* parent){
    //qDebug()<<"path:"<<path;
    NewFolderDialog* window = new NewFolderDialog(id,path,parent);
    window->setModal(true);
    window->show();
    return window;
}


void NewFolderDialog::onOk(){
    const QString name = ui->name->text();
    if(!name.isEmpty()){
        if(Utils::isFilename(name)){
            emit submit(d->id,d->path,name);

            this->close();
        }else{
            wToast::showText(tr("Invalid Folder Name"));
        }
    }
}

}
