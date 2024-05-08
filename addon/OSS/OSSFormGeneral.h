#ifndef OSSFORMGENERAL_H
#define OSSFORMGENERAL_H
#include "OSS_global.h"
#include "interface/FormPanel.h"

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
#endif // OSSFORMGENERAL_H
