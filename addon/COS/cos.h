#ifndef COS_H
#define COS_H

#include "COS_global.h"
#include "network/NetworkRequest.h"
#include "network/http/HttpClient.h"
#include <QStringList>
namespace ady {

class COS_EXPORT COS : public HttpClient
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
    constexpr const static char MARKER[] = "marker";
    constexpr const static char MAXKEYS[] = "max-keys";
    constexpr const static char PREFIX[] = "prefix";
    constexpr const static char ACCEPT_ENCODING[] = "Accept-Encoding";

    COS(CURL* curl);


    virtual NetworkResponse* link() override;
    virtual NetworkResponse* unlink() override;
    virtual NetworkResponse* listDir(QString dir,int page=1,int pageSize=1000) override;
    virtual NetworkResponse* tinyListDir(QString dir) override;
    virtual NetworkResponse* upload(Task* task) override;
    virtual NetworkResponse* download(Task* task) override;
    virtual NetworkResponse* del(Task* task) override;


    NetworkResponse* chDir(const QString &dir);
    NetworkResponse* mkDir(const QString &dir);
    NetworkResponse* rmDir(const QString &dir);
    NetworkResponse* rename(QString src,QString dst);
    NetworkResponse* del(QString bucket,QString dst);

    virtual NetworkResponse* customeAccess(QString name,QMap<QString,QVariant> data) override;


    inline void setDefaultDir(QString dir){m_defaultDir = dir;}


protected:
    QString fixUrl(QString& url,HttpParams& params);
    QString signHeaders(QString method,QString path, HttpParams& headers, HttpParams& params);
    QString urlEncode(const QString& text);
    inline unsigned char toHex(unsigned char x)
    {
        return  x > 9 ? x + 55 : x + 48;
    }

private:
    QString m_bucket;
    QString m_defaultDir;


};

}


#endif // COS_H
