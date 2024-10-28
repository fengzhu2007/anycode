#include "form_panel.h"
namespace ady {
    FormPanel::FormPanel(QWidget* parent)
        :QWidget(parent)
    {

    }

    bool FormPanel::isDataChanged()
    {
        return false;
    }

}
