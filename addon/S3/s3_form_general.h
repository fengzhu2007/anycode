#ifndef S3FORMGENERAL_H
#define S3FORMGENERAL_H
#include "s3_global.h"
#include "interface/form_panel.h"

namespace Ui {
class S3FormGeneral;
}
namespace ady {
    class S3_EXPORT S3FormGeneral : public FormPanel
    {
        Q_OBJECT
    public:
        S3FormGeneral(QWidget* parent);
        virtual void initFormData(SiteRecord record) override;
        virtual bool validateFormData(SiteRecord& record) override;

    private:
        Ui::S3FormGeneral *ui;
    };
}
#endif // S3FORMGENERAL_H
