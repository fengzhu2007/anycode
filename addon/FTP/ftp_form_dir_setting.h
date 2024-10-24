#ifndef FTP_FORM_DIR_SETTING_H
#define FTP_FORM_DIR_SETTING_H
#include "ftp_global.h"
#include "interface/FormPanel.h"
namespace Ui{
    class FTPFormDirSettingUi;
}
namespace ady {
class FTP_EXPORT FTPFormDirSetting : public FormPanel
{
    Q_OBJECT
public:
    FTPFormDirSetting(QWidget* parent=nullptr);

    virtual void initFormData(SiteRecord record) override;
    virtual bool validateFormData(SiteRecord& record) override;
    virtual bool isDataChanged() override;

private:
    Ui::FTPFormDirSettingUi* ui;
};

}


#endif // FTP_FORM_DIR_SETTING_H
