#ifndef SFTP_H
#define SFTP_H

#include "sftp_global.h"
#include "network/network_request.h"
#include <QStringList>
namespace ady {
class SFTPSetting;
class SFTP_EXPORT SFTP : public NetworkRequest
{
public:
    const int COMMAND_TIMEOUT = 5;
    const int UNLINK_TIMEOUT = 2;


    explicit SFTP(CURL* curl,long long id=0);
    ~SFTP();

    NetworkResponse* sendSyncCommand(const QString& command);
    virtual void init(const SiteRecord& info) override;
    virtual int access(NetworkResponse* response,bool body=true) override;
    virtual NetworkResponse* link() override;
    virtual NetworkResponse* unlink() override;
    virtual NetworkResponse* listDir(const QString& dir,int page=1,int pageSize=1000) override;
    virtual NetworkResponse* tinyListDir(const QString& dir) override;
    virtual NetworkResponse* upload(Task* task) override;
    virtual NetworkResponse* download(Task* task) override;
    virtual NetworkResponse* del(Task* task) override;
    virtual NetworkResponse* chmod(Task* task) override;


    //NetworkResponse* setAscii();
    //NetworkResponse* setBinary();
    //NetworkResponse* setPassive(bool pasv);
    //NetworkResponse* chDir(const QString &dir);
    virtual NetworkResponse* mkDir(const QString &dir) override;
    virtual NetworkResponse* rmDir(const QString &dir) override;
    NetworkResponse* pwd();
    virtual NetworkResponse* rename(const QString& src,const QString& dst) override;
    virtual NetworkResponse* chmod(const QString& dst,int mode) override;
    virtual NetworkResponse* del(const QString& dst) override;


    void setUploadCommands(QStringList commands);



    virtual QString matchToPath(const QString& from,bool is_file,bool local) override;
    virtual NetworkResponse* customeAccess(const QString& name,QMap<QString,QVariant> data) override;


protected:
    void initEnv();
    NetworkResponse* sendCommand(const QString& command);
    QString fixedCommandPath(QString &command,const QString &path);



private:
    QStringList m_uploadCommands;
    SFTPSetting* m_setting;
    QList<QPair<QString,QString>> m_dirMapping;
    QString m_rootPath;


};

}


#endif // SFTP_H
