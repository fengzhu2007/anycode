#ifndef NEW_PROJECT_ZERO_WIDGET_H
#define NEW_PROJECT_ZERO_WIDGET_H

#include <QWidget>

namespace Ui {
class NewProjectZeroWidget;
}
namespace ady {
    class NewProjectZeroWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit NewProjectZeroWidget(QWidget *parent = nullptr);
        ~NewProjectZeroWidget();
        void initData();
    public slots:
        void onNewProject();
        void onProjectItemClicked(int i);
        void onProjectEditClicked(int i);

    private:
        Ui::NewProjectZeroWidget *ui;
    };
}
#endif // NEW_PROJECT_ZERO_WIDGET_H
