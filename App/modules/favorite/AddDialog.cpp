#include "AddDialog.h"
#include "Ui_AddDialog.h"
#include "storage/FavoriteStorage.h"
#include "components/MessageDialog.h"
namespace ady {
    FavoriteAddDialog::FavoriteAddDialog(QWidget* parent,long long id)
        :QDialog(parent),
        ui(new Ui::FavoriteAddDialog){

            ui->setupUi(this);
            connect(ui->okButton,&QPushButton::clicked,this,&FavoriteAddDialog::onSave);
            connect(ui->cancelButton,&QPushButton::clicked,this,&FavoriteAddDialog::close);
            m_id = id;

    }

    FavoriteAddDialog::~FavoriteAddDialog(){
        delete ui;
    }

    void FavoriteAddDialog::setCurrentPath(QString path)
    {
        ui->pathLineEdit->setText(path);
    }

    void FavoriteAddDialog::onSave()
    {
        FavoriteStorage db;
        FavoriteRecord r;
        r.pid = m_id;
        r.name = ui->nameLineEdit->text();
        if(r.name.isEmpty()){
            MessageDialog::info(this,tr("Name is required!"));
            return ;
        }
        r.path = ui->pathLineEdit->text();
        r.sid = 0ll;
        long long id = db.insert(r);
        if(id>0){
            emit onSaved();

            MessageDialog::info(this,tr("Save successfully!"));
            close();
        }else{
            MessageDialog::info(this,tr("Save failed!"));
        }

    }
}
