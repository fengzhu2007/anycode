#ifndef FTP_H
#define FTP_H

#include "ftp_global.h"
#include "network/network_request.h"
#include <QStringList>
namespace ady {
class FTPSetting;
class FTP_EXPORT FTP : public NetworkRequest
{
public:
    const int COMMAND_TIMEOUT = 5;
    const int UNLINK_TIMEOUT = 2;


    explicit FTP(CURL* curl,long long id=0);
    virtual ~FTP();

    NetworkResponse* sendSyncCommand(const QString& command,bool pre_utf8=true);

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

    virtual NetworkResponse* del(const QString& path) override;
    virtual NetworkResponse* mkDir(const QString &dir) override ;
    virtual NetworkResponse* rmDir(const QString &dir) override ;
    virtual NetworkResponse* rename(const QString& src,const QString& dst) override;
    virtual NetworkResponse* chmod(const QString& dst,int mode) override;

    virtual QString matchToPath(const QString& from,bool local) override;
    virtual void autoClose(long long current) override;


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

    FTPSetting* m_setting;
    //QList<QPair<QString,QString>> m_dirSync;
    QList<QPair<QString,QString>> m_dirMapping;
    QString m_rootPath;


    //QStringList m_filterDirs;
    //QStringList m_filterExts;

    //QList<QRegularExpression> m_allowextensions;


};

}


#endif // FTP_H
