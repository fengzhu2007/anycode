#ifndef {TPL}FORMGENERAL_H
#define {TPL}FORMGENERAL_H
#include "{tpl}_global.h"
#include "interface/form_panel.h"

namespace Ui {
class {TPL}FormGeneralUI;
}
namespace ady {
    class {TPL}_EXPORT {TPL}FormGeneral : public FormPanel
    {
        Q_OBJECT
    public:
        {TPL}FormGeneral(QWidget* parent);
        virtual void initFormData(SiteRecord record) override;
        virtual bool validateFormData(SiteRecord& record) override;

    private:
        Ui::{TPL}FormGeneralUI *ui;
    };
}
#endif // {TPL}FORMGENERAL_H
