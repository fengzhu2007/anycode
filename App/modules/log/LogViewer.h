#ifndef LOGVIEWER_H
#define LOGVIEWER_H


#include <QDialog>

namespace Ui {
    class LogViewer;
}


namespace ady {
    class LogViewer : public QDialog
    {
        Q_OBJECT
    public:
        LogViewer(QWidget* parent);
        ~LogViewer();
        void scrollBottom();

    public slots:
        void onDelete();
        void onClearAll();


    private:
        Ui::LogViewer* ui;


    };
}

#endif // LOGVIEWER_H
