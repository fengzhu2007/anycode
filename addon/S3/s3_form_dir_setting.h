#ifndef S3FORMDIRSETTING_H
#define S3FORMDIRSETTING_H
#include "s3_global.h"
#include "interface/form_panel.h"
namespace Ui{
    class S3FormDirSetting;
}
namespace ady {
class S3_EXPORT S3FormDirSetting : public FormPanel
{
    Q_OBJECT
public:
    S3FormDirSetting(QWidget* parent=nullptr);

    virtual void initFormData(SiteRecord record) override;
    virtual bool validateFormData(SiteRecord& record) override;
    virtual bool isDataChanged() override;

private:
    Ui::S3FormDirSetting* ui;
};

}


#endif // S3FORMDIRSETTING_H
