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
        static void error(QWidget* parent,const QString& title);
        static void info(QWidget* parent,const QString& title);
        static int confirm(QWidget* parent,const QString& title,QMessageBox::StandardButtons buttons = QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));
        static int confirm(QWidget* parent,const QString& title,const QString& text,QMessageBox::StandardButtons buttons = QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));




    };
}
#endif // MESSAGEBOX_H
