#include "message_dialog.h"
namespace ady {


void MessageDialog::error(QWidget* parent,const QString& title)
{
    QMessageBox::warning(parent,QObject::tr("Error"),title);
}

void MessageDialog::info(QWidget* parent,const QString& title)
{
    QMessageBox::information(parent,QObject::tr("Information"),title);
}

int MessageDialog::confirm(QWidget* parent,const QString& title,QMessageBox::StandardButtons buttons )
{
    return QMessageBox::question(parent,QObject::tr("Confirm"),title,buttons);
}

int MessageDialog::confirm(QWidget* parent,const QString& title,const QString& text,QMessageBox::StandardButtons buttons){
    return QMessageBox::question(parent,title,text,buttons);
}

}
