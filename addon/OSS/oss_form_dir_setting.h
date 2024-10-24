#ifndef OSSFormDirSETTING_H
#define OSSFormDirSETTING_H
#include "oss_global.h"
#include "interface/FormPanel.h"
namespace Ui{
    class OSSFormDirSettingUi;
}
namespace ady {
class OSS_EXPORT OSSFormDirSetting : public FormPanel
{
    Q_OBJECT
public:
    OSSFormDirSetting(QWidget* parent=nullptr);

    virtual void initFormData(SiteRecord record) override;
    virtual bool validateFormData(SiteRecord& record) override;
    virtual bool isDataChanged() override;

private:
    Ui::OSSFormDirSettingUi* ui;
};

}


#endif // OSSFormDirSETTING_H
