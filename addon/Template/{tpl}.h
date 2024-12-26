#ifndef {TPL}_H
#define {TPL}_H

#include "{tpl}_global.h"
#include "network/network_request.h"
#include <QStringList>
namespace ady {

class {TPL}_EXPORT {TPL} : public NetworkRequest
{
public:
    const int COMMAND_TIMEOUT = 5;
    const int UNLINK_TIMEOUT = 2;


    {TPL}(CURL* curl);

    NetworkResponse* sendSyncCommand(QString command);

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
    NetworkResponse* sendCommand(QString command);



private:
    QStringList feats;
    bool mlsd;
    bool mfmt;
    bool utf8;


};

}


#endif // {TPL}_H
