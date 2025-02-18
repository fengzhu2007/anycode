#ifndef S3_H
#define S3_H

#include "s3_global.h"
#include "network/network_request.h"
#include "network/http/http_client.h"
#include "s3_setting.h"
namespace ady {

class S3_EXPORT S3 : public HttpClient
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
    constexpr const static char DELIMITER[] = "delimiter";
    constexpr const static char CONTINUATION_TOKEN[] = "continuation-token";
    constexpr const static char MAXKEYS[] = "max-keys";
    constexpr const static char PREFIX[] = "prefix";
    constexpr const static char ACCEPT_ENCODING[] = "Accept-Encoding";
    constexpr const static char LIST_TYPE[] = "list-type";
    constexpr const static char CONTENT_SHA256[] = "x-amz-content-sha256";
    constexpr const static char EMPTY_HASH[] = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";


    S3(CURL* curl,long long id=0);
    ~S3();

    virtual void init(const SiteRecord& info) override;

    virtual NetworkResponse* link() override;
    virtual NetworkResponse* unlink() override;
    virtual NetworkResponse* listDir(const QString& dir,int page=1,int pageSize=1000) override;
    virtual NetworkResponse* tinyListDir(const QString& dir) override;
    virtual NetworkResponse* upload(Task* task) override;
    virtual NetworkResponse* download(Task* task) override;
    virtual NetworkResponse* del(Task* task) override;
    virtual NetworkResponse* del(const QString &dst) override;

    virtual NetworkResponse* mkDir(const QString &dir) override;
    virtual NetworkResponse* rmDir(const QString &dir) override;
    NetworkResponse* del(const QString& bucket,const QString& dst);


    virtual NetworkResponse* customeAccess(const QString& name,QMap<QString,QVariant> data) override;
    virtual QString matchToPath(const QString& from,bool is_file,bool local) override;

    inline void setDefaultDir(QString dir){m_rootPath = dir;}

protected:
    QString fixUrl(QString& url,HttpParams& params);
    QString signHeaders(QString method,QString path, HttpParams& headers, HttpParams& params,const QString& hashPayload=EMPTY_HASH);
    QString urlEncode(const QString& text);
    inline unsigned char toHex(unsigned char x)
    {
        return  x > 9 ? x + 55 : x + 48;
    }



private:
    QString m_bucket;
    QString m_rootPath;
    S3Setting* m_setting;
    QList<QPair<QString,QString>> m_dirMapping;
    QList<QRegularExpression> m_filters;

};

}


#endif // S3_H
