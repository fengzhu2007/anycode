#ifndef QUICK_PANE_IMPL_H
#define QUICK_PANE_IMPL_H

#include <docking_pane.h>

namespace ady{
class QuickPaneImpl : public DockingPane
{
public:
    explicit QuickPaneImpl(QWidget *parent = nullptr);
};
}
#endif // QUICK_PANE_IMPL_H
