#include "open_project_window.h"
#include "ui_open_project_window.h"
#include "ProjectListModel.h"
#include "components/MessageDialog.h"
#include <QDebug>
namespace ady {
OpenProjectWindow::OpenProjectWindow(QWidget* parent)
    :wDialog(parent),
     ui(new Ui::OpenProjectWindow)
{

    ui->setupUi(this);

    this->resetupUi();

    connect(ui->okButton,&QPushButton::clicked,this,&OpenProjectWindow::onSelected);
    connect(ui->cancelButton,&QPushButton::clicked,this,&OpenProjectWindow::close);


    this->initData();
}

void OpenProjectWindow::initData()
{
    ProjectStorage projectStorage;
    QList<ProjectRecord>lists = projectStorage.all();
    ProjectListModel* model = new ProjectListModel(ui->treeView);
    model->setData(lists);
    ui->treeView->setModel(model);
}

OpenProjectWindow* OpenProjectWindow::open(QWidget* parent){
    OpenProjectWindow* window = new OpenProjectWindow(parent);
    //window->setWindowModality(Qt::ApplicationModal);
    window->setModal(true);
    window->show();
    return window;
}

void OpenProjectWindow::onSelected()
{
   QItemSelectionModel *m = ui->treeView->selectionModel();
   if(m!=nullptr && m->hasSelection()){
       //qDebug()<<"has selection:"<<m->hasSelection();
       QModelIndex index = m->currentIndex();
       int row = index.row();
       //qDebug()<<"row:"<<row;
       ProjectListModel* model = static_cast<ProjectListModel*>(ui->treeView->model());
       if(row>=0 && row < model->rowCount()){
            ProjectRecord record = model->dataIndex(row);
            emit selectionChanged(record.id);
            close();
       }
   }else{
        MessageDialog::error(this,tr("Please select project"));
   }
}

}
