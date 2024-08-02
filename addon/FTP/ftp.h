#ifndef FTP_H
#define FTP_H

#include "FTP_global.h"
#include "network/network_request.h"
#include <QStringList>
namespace ady {

class FTP_EXPORT FTP : public NetworkRequest
{
public:
    const int COMMAND_TIMEOUT = 5;
    const int UNLINK_TIMEOUT = 2;


    FTP(CURL* curl,long long id=0);

    NetworkResponse* sendSyncCommand(const QString& command,bool pre_utf8=true);

    virtual int access(NetworkResponse* response,bool body=true) override;
    virtual NetworkResponse* link() override;
    virtual NetworkResponse* unlink() override;
    virtual NetworkResponse* listDir(const QString& dir,int page=1,int pageSize=1000) override;
    virtual NetworkResponse* tinyListDir(const QString& dir) override;
    virtual NetworkResponse* upload(Task* task) override;
    virtual NetworkResponse* download(Task* task) override;
    virtual NetworkResponse* del(Task* task) override;
    virtual NetworkResponse* chmod(Task* task) override;

    virtual NetworkResponse* del(const QString& path) override;
    virtual NetworkResponse* mkDir(const QString &dir) override ;
    virtual NetworkResponse* rmDir(const QString &dir) override ;
    virtual NetworkResponse* rename(const QString& src,const QString& dst) override;
    virtual NetworkResponse* chmod(const QString& dst,int mode) override;


    NetworkResponse* setAscii();
    NetworkResponse* setBinary();
    NetworkResponse* setPassive(bool pasv);
    NetworkResponse* chDir(const QString &dir);

    NetworkResponse* pwd();






    virtual NetworkResponse* customeAccess(const QString& name,QMap<QString,QVariant> data) override;

protected:
    void initEnv();
    NetworkResponse* sendCommand(const QString& command,bool pre_utf8=true);



private:
    QStringList feats;
    bool mlsd;
    bool mfmt;
    bool utf8;
    bool window_nt;


};

}


#endif // FTP_H
