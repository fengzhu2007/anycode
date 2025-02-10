#ifndef SSL_QUERY_DIALOG_H
#define SSL_QUERY_DIALOG_H

#include <w_dialog.h>

namespace Ui{
class SSLQueryDialog;
}
namespace ady{
class SSLQueryDialogPrivate;
class SSLQueryDialog : public wDialog
{
    Q_OBJECT
public:
    ~SSLQueryDialog();
    static SSLQueryDialog* getInstance();
    static SSLQueryDialog* open(QWidget* parent);

public slots:
    void onQuery();

private:
    explicit SSLQueryDialog(QWidget* parent=nullptr);
private:
    Ui::SSLQueryDialog* ui;
    SSLQueryDialogPrivate* d;
    static SSLQueryDialog* instance;
};

}
#endif // SSL_QUERY_DIALOG_H
