#ifndef FTP_H
#define FTP_H

#include "FTP_global.h"
#include "network/NetworkRequest.h"
#include <QStringList>
namespace ady {

class FTP_EXPORT FTP : public NetworkRequest
{
public:
    const int COMMAND_TIMEOUT = 5;
    const int UNLINK_TIMEOUT = 2;


    FTP(CURL* curl);

    NetworkResponse* sendSyncCommand(QString command,bool pre_utf8=true);

    virtual int access(NetworkResponse* response,bool body=true) override;
    virtual NetworkResponse* link() override;
    virtual NetworkResponse* unlink() override;
    virtual NetworkResponse* listDir(QString dir,int page=1,int pageSize=1000) override;
    virtual NetworkResponse* tinyListDir(QString dir) override;
    virtual NetworkResponse* upload(Task* task) override;
    virtual NetworkResponse* download(Task* task) override;
    virtual NetworkResponse* del(Task* task) override;
    virtual NetworkResponse* chmod(Task* task) override;


    NetworkResponse* setAscii();
    NetworkResponse* setBinary();
    NetworkResponse* setPassive(bool pasv);
    NetworkResponse* chDir(const QString &dir);
    NetworkResponse* mkDir(const QString &dir);
    NetworkResponse* rmDir(const QString &dir);
    NetworkResponse* pwd();
    NetworkResponse* rename(QString src,QString dst);
    NetworkResponse* chmod(QString dst,int mode);
    NetworkResponse* del(QString dst);





    virtual NetworkResponse* customeAccess(QString name,QMap<QString,QVariant> data) override;

protected:
    void initEnv();
    NetworkResponse* sendCommand(QString command,bool pre_utf8=true);



private:
    QStringList feats;
    bool mlsd;
    bool mfmt;
    bool utf8;
    bool window_nt;


};

}


#endif // FTP_H