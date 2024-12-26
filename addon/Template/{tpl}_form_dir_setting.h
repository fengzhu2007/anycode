#ifndef {TPL}FORMDIRSETTING_H
#define {TPL}FORMDIRSETTING_H
#include "{tpl}_global.h"
#include "interface/form_panel.h"
namespace Ui{
    class {TPL}FormDirSettingUi;
}
namespace ady {
class {TPL}_EXPORT {TPL}FormDirSetting : public FormPanel
{
    Q_OBJECT
public:
    {TPL}FormDirSetting(QWidget* parent=nullptr);

    virtual void initFormData(SiteRecord record) override;
    virtual bool validateFormData(SiteRecord& record) override;
    virtual bool isDataChanged() override;

private:
    Ui::{TPL}FormDirSettingUi* ui;
};

}


#endif // {TPL}FORMDIRSETTING_H
