#include "FormPanel.h"
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
