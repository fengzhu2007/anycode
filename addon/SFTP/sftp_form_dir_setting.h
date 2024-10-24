#ifndef SFTPFORMDIRSETTING_H
#define SFTPFORMDIRSETTING_H
#include "sftp_global.h"
#include "interface/FormPanel.h"
namespace Ui{
    class SFTPFormDirSettingUi;
}
namespace ady {
class SFTP_EXPORT SFTPFormDirSetting : public FormPanel
{
    Q_OBJECT
public:
    SFTPFormDirSetting(QWidget* parent=nullptr);

    virtual void initFormData(SiteRecord record) override;
    virtual bool validateFormData(SiteRecord& record) override;
    virtual bool isDataChanged() override;

private:
    Ui::SFTPFormDirSettingUi* ui;
};

}


#endif // SFTPFORMDIRSETTING_H
