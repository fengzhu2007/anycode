#ifndef COSFORMGENERAL_H
#define COSFORMGENERAL_H
#include "cos_global.h"
#include "interface/FormPanel.h"

namespace Ui {
class COSFormGeneralUI;
}
namespace ady {
    class COS_EXPORT COSFormGeneral : public FormPanel
    {
        Q_OBJECT
    public:
        COSFormGeneral(QWidget* parent);
        virtual void initFormData(SiteRecord record) override;
        virtual bool validateFormData(SiteRecord& record) override;

    private:
        Ui::COSFormGeneralUI *ui;
    };
}
#endif // COSFORMGENERAL_H
