#ifndef ERROR_PANE_H
#define ERROR_PANE_H

#include <docking_pane.h>

namespace Ui {
class ErrorPane;
}

namespace ady{
class ErrorPanePrivate;
class ErrorPane : public DockingPane
{
    Q_OBJECT

public:
    explicit ErrorPane(QWidget *parent = nullptr);
    ~ErrorPane();
    virtual QString id() override;
    virtual QString group() override;

    static ErrorPane* make(DockingPaneManager* dockingManager,const QJsonObject& data);

public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;

private:
    Ui::ErrorPane *ui;
    ErrorPanePrivate* d;
};
}

#endif // ERROR_PANE_H
