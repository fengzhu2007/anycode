#ifndef SPLITTERHANDLEWIDGET_H
#define SPLITTERHANDLEWIDGET_H
#include <QWidget>
#include <QAction>

namespace Ui {
class SplitterHandleWidgetUI;
}

namespace ady {
    class SplitterHandleWidget : public QWidget
    {
        Q_OBJECT

    public:
        const static int A_NEW_PROJECT = 1;
        const static int A_DEL_PROJECT = 2;
        const static int A_NEW_SITE = 3;
        const static int A_DEL_SITE = 4;
        const static int A_MOVE_UP = 5;
        const static int A_MOVE_DOWN = 6;


    public:
        explicit SplitterHandleWidget(QWidget *parent = nullptr);
        ~SplitterHandleWidget();
        void setActionGroupStatus(int role);
    public slots:
        void onTriggered(bool checked);

    signals:
        void actionTriggered(QAction* ,bool);


    private:
        Ui::SplitterHandleWidgetUI *ui;
    };
}
#endif // SPLITTERHANDLEWIDGET_H
