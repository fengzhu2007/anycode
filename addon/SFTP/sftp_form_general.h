#ifndef SFTPFORMGENERAL_H
#define SFTPFORMGENERAL_H
#include "sftp_global.h"
#include "interface/FormPanel.h"

namespace Ui {
class SFTPFormGeneralUI;
}
namespace ady {
    class SFTP_EXPORT SFTPFormGeneral : public FormPanel
    {
        Q_OBJECT
    public:
        SFTPFormGeneral(QWidget* parent);
        virtual void initFormData(SiteRecord record) override;
        virtual bool validateFormData(SiteRecord& record) override;

    private:
        Ui::SFTPFormGeneralUI *ui;
    };
}
#endif // SFTPFORMGENERAL_H
