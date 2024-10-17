#ifndef STARTUPPANE_H
#define STARTUPPANE_H

#include <docking_pane.h>
namespace ady{
class StartupPane : public DockingPane
{
public:
    explicit StartupPane(QWidget *parent = nullptr);
    virtual ~StartupPane();
    virtual QString id() override;
    virtual QString group() override;
    virtual QString description() override;


public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;


private:


};
}
#endif // STARTUPPANE_H
