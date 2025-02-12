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
    void onActionTrigger();
    void onQuery();
    void onOneReady(const QString& domain,const QJsonObject& data);
    void onOneError(const QString& domain,const QString& errorMsg);
    void onFinish();

private:
    explicit SSLQueryDialog(QWidget* parent=nullptr);
    QDateTime parseDateTime(const QString& dateTimeString);
    void queryDomains(const QStringList& sites);
private:
    Ui::SSLQueryDialog* ui;
    SSLQueryDialogPrivate* d;
    static SSLQueryDialog* instance;
};

}
#endif // SSL_QUERY_DIALOG_H
