#ifndef FIND_REPLACE_PANE_H
#define FIND_REPLACE_PANE_H

#include <QWidget>

namespace Ui {
class FindReplacePane;
}

class FindReplacePane : public QWidget
{
    Q_OBJECT

public:
    explicit FindReplacePane(QWidget *parent = nullptr);
    ~FindReplacePane();

private:
    Ui::FindReplacePane *ui;
};

#endif // FIND_REPLACE_PANE_H
