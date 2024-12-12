#ifndef PROJECT_WIDGET_H
#define PROJECT_WIDGET_H

#include <QWidget>

namespace Ui {
class ProjectWidget;
}

class ProjectWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectWidget(QWidget *parent = nullptr);
    ~ProjectWidget();

private:
    Ui::ProjectWidget *ui;
};

#endif // PROJECT_WIDGET_H
