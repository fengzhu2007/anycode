#ifndef OSS_H
#define OSS_H
#include "oss_global.h"
#include "network/network_request.h"
#include "network/http/http_client.h"




namespace ady {
class OSSSetting;
class OSS_EXPORT OSS : public HttpClient
{
public:
    const int COMMAND_TIMEOUT = 5;
    const int UNLINK_TIMEOUT = 2;

    constexpr const static char METHOD[] = "method";
    constexpr const static char OBJECT[] = "object";
    constexpr const static char BUCKET[] = "bucket";
    constexpr const static char CONTENT_TYPE[] = "Content-Type";
    constexpr const static char CONTENT_MD5[] = "Content-Md5";
    constexpr const static char DATE[] = "Date";
    constexpr const static char HOST[] = "Host";
    constexpr const static char CONTENT_LENGTH[] = "Content-Length";
    constexpr const static char AUTHORIZATION[] = "Authorization";
    constexpr const static char DELIMITER[] = "Delimiter";
    constexpr const static char MARKER[] = "Marker";
    constexpr const static char MAXKEYS[] = "Max-keys";
    constexpr const static char PREFIX[] = "Prefix";
    constexpr const static char ACCEPT_ENCODING[] = "Accept-Encoding";



    OSS(CURL* curl,long long id=0);

    //virtual int access(NetworkResponse* response,bool body=true) override;
    virtual void init(const SiteRecord& info) override;
    virtual NetworkResponse* link() override;
    virtual NetworkResponse* unlink() override;
    virtual NetworkResponse* listDir(const QString& dir,int page=1,int pageSize=1000) override;
    virtual NetworkResponse* tinyListDir(const QString& dir) override;
    virtual NetworkResponse* upload(Task* task) override;
    virtual NetworkResponse* download(Task* task) override;
    virtual NetworkResponse* del(Task* task) override;
    virtual NetworkResponse* del(const QString& dst) override;

    //virtual NetworkResponse* chDir(const QString &dir) override;
    virtual NetworkResponse* mkDir(const QString &dir) override;
    virtual NetworkResponse* rmDir(const QString &dir) override;




    NetworkResponse* del(const QString& bucket,const QString& dst);



    virtual NetworkResponse* customeAccess(const QString& name,QMap<QString,QVariant> data) override;
    virtual QString matchToPath(const QString& from,bool local) override;


    inline void setDefaultDir(const QString& dir){m_rootPath = dir;}



protected:
    HttpParams signHeaders(const HttpParams& options);


private:
    QString m_bucket;
    QString m_rootPath;

    OSSSetting* m_setting;
    QList<QPair<QString,QString>> m_dirMapping;
    QList<QRegularExpression> m_filters;

};
}
#endif // OSS_H
