#ifndef OSS_H
#define OSS_H
#include "OSS_global.h"
#include "network/NetworkRequest.h"
#include "network/http/HttpClient.h"




namespace ady {

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



    OSS(CURL* curl);

    //virtual int access(NetworkResponse* response,bool body=true) override;
    virtual NetworkResponse* link() override;
    virtual NetworkResponse* unlink() override;
    virtual NetworkResponse* listDir(QString dir,int page=1,int pageSize=1000) override;
    virtual NetworkResponse* tinyListDir(QString dir) override;
    virtual NetworkResponse* upload(Task* task) override;
    virtual NetworkResponse* download(Task* task) override;
    virtual NetworkResponse* del(Task* task) override;
    //virtual NetworkResponse* chmod(Task* task) override;


    NetworkResponse* chDir(const QString &dir);
    NetworkResponse* mkDir(const QString &dir);
    NetworkResponse* rmDir(const QString &dir);
    NetworkResponse* rename(QString src,QString dst);
    NetworkResponse* del(QString bucket,QString dst);



    virtual NetworkResponse* customeAccess(QString name,QMap<QString,QVariant> data) override;


    inline void setDefaultDir(QString dir){m_defaultDir = dir;}


protected:
    HttpParams signHeaders(const HttpParams& options);


private:
    QString m_bucket;
    QString m_defaultDir;

};
}
#endif // OSS_H
