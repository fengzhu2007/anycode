#ifndef DBMS_PANE_H
#define DBMS_PANE_H

#include <QWidget>

namespace Ui {
class DBMSPane;
}

class DBMSPane : public QWidget
{
    Q_OBJECT

public:
    explicit DBMSPane(QWidget *parent = nullptr);
    ~DBMSPane();

private:
    Ui::DBMSPane *ui;
};

#endif // DBMS_PANE_H
