#include "MessageDialog.h"
namespace ady {


void MessageDialog::error(QWidget* parent,QString title)
{
    QMessageBox::warning(parent,QObject::tr("Error"),title);
}

void MessageDialog::info(QWidget* parent,QString title)
{
    QMessageBox::information(parent,QObject::tr("Information"),title);
}

int MessageDialog::confirm(QWidget* parent,QString title,QMessageBox::StandardButtons buttons )
{
    return QMessageBox::question(parent,QObject::tr("Confirm"),title,buttons);
}

}
