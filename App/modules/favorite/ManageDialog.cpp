#include "ManageDialog.h"
#include "ui_ManageDialog.h"
#include "FavoriteItemModel.h"
#include "components/MessageDialog.h"
#include <QMenu>
#include <QItemSelectionModel>
namespace ady {
    FavoriteManageDialog::FavoriteManageDialog(QWidget* parent ,long long id)
        :QDialog(parent),
        ui(new Ui::FavoriteManageDialog){

        ui->setupUi(this);
        FavoriteStorage db;
        QList<FavoriteRecord>lists = db.list(id);
        FavoriteItemModel* model = new FavoriteItemModel(ui->treeView);
        model->setList(lists);
        ui->treeView->setModel(model);
        //ui->treeView->setEditTriggers(QAbstractItemView::DoubleClicked);
        connect(model,SIGNAL(rename(const QModelIndex&,QString)),this,SLOT(onRename(const QModelIndex&,QString)));
        connect(ui->closeButton,&QPushButton::clicked,this,&FavoriteManageDialog::close);

        ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->treeView,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(onContextMenu(const QPoint&)));
    }

    void FavoriteManageDialog::onRename(const QModelIndex& index,QString name)
    {
        FavoriteItemModel* model = (FavoriteItemModel*)ui->treeView->model();
        FavoriteRecord r = model->getItem(index.row());
        r.name = name;
        FavoriteStorage db;
        if(db.update(r)>0){
            model->updateItem(index.row(),r);
            emit onSaved();
            MessageDialog::error(this,tr("Save successfully"));
        }else{
            MessageDialog::error(this,tr("Save failed"));
        }
    }

    void FavoriteManageDialog::onContextMenu(const QPoint& point)
    {
        Q_UNUSED(point);
        QMenu* menu = new QMenu;
        menu->addAction(tr("Delete"),this,&FavoriteManageDialog::onDelete);
        menu->exec(QCursor::pos());
    }

    void FavoriteManageDialog::onDelete(){
        if(MessageDialog::confirm(this,tr("Are you want to delete selected item?"),QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes){
            QItemSelectionModel* m = ui->treeView->selectionModel();
            if(m!=nullptr){
                QModelIndexList lists = m->selectedRows();
                if(lists.size()>0){
                    FavoriteStorage db;
                     FavoriteItemModel* model = (FavoriteItemModel*)ui->treeView->model();
                     Q_FOREACH(QModelIndex index,lists){
                        db.del(model->getItem(index.row()).id);
                        model->removeItem(index.row());
                     }
                     emit onSaved();
                     MessageDialog::error(this,tr("Delete successfully"));
                }
            }
        }
    }

    FavoriteManageDialog::~FavoriteManageDialog(){
        delete ui;
    }


}
