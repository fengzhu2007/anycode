#ifndef SESSION_CONFIG_DIALOG_H
#define SESSION_CONFIG_DIALOG_H

#include "w_dialog.h"

namespace Ui {
class SessionConfigDialog;
}

namespace ady {

class SessionConfigDialog : public wDialog
{
    Q_OBJECT
public:
    explicit SessionConfigDialog(QWidget *parent = nullptr);
    ~SessionConfigDialog();

    void setSessionTitle(const QString &title);
    void setPreference(const QString &preference);
    void setDirectory(const QString &dir);
    void setDirectoryList(const QStringList &paths);

    QString sessionTitle() const;
    QString preference() const;
    QString directory() const;

private:
    Ui::SessionConfigDialog *ui;
};

}

#endif // SESSION_CONFIG_DIALOG_H
