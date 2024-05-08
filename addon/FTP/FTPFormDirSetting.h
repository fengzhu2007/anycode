#ifndef FTPFORMDIRSETTING_H
#define FTPFORMDIRSETTING_H
#include "FTP_global.h"
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


#endif // FTPFORMDIRSETTING_H
