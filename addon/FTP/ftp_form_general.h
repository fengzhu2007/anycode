#ifndef FTP_FORM_GENERAL_H
#define FTP_FORM_GENERAL_H
#include "ftp_global.h"
#include "interface/FormPanel.h"

namespace Ui {
class FTPFormGeneralUI;
}
namespace ady {
    class FTP_EXPORT FTPFormGeneral : public FormPanel
    {
        Q_OBJECT
    public:
        FTPFormGeneral(QWidget* parent);
        virtual void initFormData(SiteRecord record) override;
        virtual bool validateFormData(SiteRecord& record) override;

    private:
        Ui::FTPFormGeneralUI *ui;
    };
}
#endif // FTP_FORM_GENERAL_H
