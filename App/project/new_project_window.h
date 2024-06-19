#ifndef NEW_PROJECT_WINDOW_H
#define NEW_PROJECT_WINDOW_H
#include "global.h"
#include "w_dialog.h"
#include <QDialog>

namespace Ui {
class NewProjectWindow;
}

namespace ady{
    class NewProjectWindowPrivate;
    class ANYENGINE_EXPORT NewProjectWindow : public wDialog
    {
        Q_OBJECT

    public:
        explicit NewProjectWindow(QWidget *parent = nullptr);
        ~NewProjectWindow();
        void setCurrentIndex(int i);
        void setCurrentProjectId(long long id);
        long long currentProjectId();
        void setOrigin(int i);
    public slots:
        void next();
        void previous();

        static NewProjectWindow* open(QWidget* parent);




    private:
        Ui::NewProjectWindow *ui;
        NewProjectWindowPrivate *d;
    };
}

#endif // NEW_PROJECT_WINDOW_H
