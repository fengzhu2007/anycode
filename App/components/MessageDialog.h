#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H
#include <QMessageBox>
#include "global.h"
#include <QString>
#include <QWidget>
namespace ady {
    class ANYENGINE_EXPORT MessageDialog
    {
    public:
        static void error(QWidget* parent,QString title);
        static void info(QWidget* parent,QString title);
        static int confirm(QWidget* parent,QString title,QMessageBox::StandardButtons buttons = QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));




    };
}
#endif // MESSAGEBOX_H
