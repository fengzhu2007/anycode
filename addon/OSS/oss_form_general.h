#ifndef OSS_FORM_GENERAL_H
#define OSS_FORM_GENERAL_H
#include "oss_global.h"
#include "interface/form_panel.h"

namespace Ui {
class OSSFormGeneralUI;
}
namespace ady {
    class OSS_EXPORT OSSFormGeneral : public FormPanel
    {
        Q_OBJECT
    public:
        OSSFormGeneral(QWidget* parent);
        virtual void initFormData(SiteRecord record) override;
        virtual bool validateFormData(SiteRecord& record) override;

    private:
        Ui::OSSFormGeneralUI *ui;
    };
}
#endif // OSS_FORM_GENERAL_H
