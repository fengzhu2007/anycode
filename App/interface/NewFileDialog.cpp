#include "NewFileDialog.h"
#include "ui_NewFileDialog.h"
#include "utils.h"
#include "components/MessageDialog.h"
#include <QFileInfo>
#include <QDebug>
namespace ady {
    NewFileDialog::NewFileDialog(Type type,QWidget* parent)
        :QDialog(parent),
        ui(new Ui::NewFileDialog())
    {
        ui->setupUi(this);
        if(type==File){
            setWindowTitle(tr("New File"));
        }else if(type==Folder){
            setWindowTitle(tr("New Folder"));
        }
        connect(ui->okButton,&QPushButton::clicked,this,&QDialog::accept);
        connect(ui->cancelButton,&QPushButton::clicked,this,&QDialog::reject);
        connect(ui->nameLineEdit,SIGNAL(textChanged(const QString&)),this,SLOT(onNameChanged(const QString&)));

    }

    NewFileDialog::~NewFileDialog()
    {
        delete ui;
    }

    void NewFileDialog::onOK()
    {
        if(Utils::isFilename(m_name)){
            if(QFileInfo::exists(fileName())){
                MessageDialog::error(this,tr("File or Folder is already exists"));
            }else{
                this->accept();
            }
        }else{
            MessageDialog::error(this,tr("Invalid filename"));
        }
    }

    void NewFileDialog::onNameChanged(const QString& name)
    {
        m_name = name;
    }


    QString NewFileDialog::fileName()
    {
        QString filename = m_path;
        if(!filename.endsWith("/")){
            filename += "/";
        }
        return filename += m_name;
    }

    void NewFileDialog::setName(QString name)
    {
        m_name = name;
        ui->nameLineEdit->setText(m_name);
    }

    void NewFileDialog::setPath(QString path)
    {
        m_path = path;
        ui->pathLineEdit->setText(m_path);
        qDebug()<<"path:"<<path;
    }


}
