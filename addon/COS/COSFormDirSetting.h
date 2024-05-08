#ifndef COSFormDirSETTING_H
#define COSFormDirSETTING_H
#include "COS_global.h"
#include "interface/FormPanel.h"
namespace Ui{
    class COSFormDirSettingUi;
}
namespace ady {
class COS_EXPORT COSFormDirSetting : public FormPanel
{
    Q_OBJECT
public:
    COSFormDirSetting(QWidget* parent=nullptr);

    virtual void initFormData(SiteRecord record) override;
    virtual bool validateFormData(SiteRecord& record) override;
    virtual bool isDataChanged() override;

private:
    Ui::COSFormDirSettingUi* ui;
};

}


#endif // COSFormDirSETTING_H
