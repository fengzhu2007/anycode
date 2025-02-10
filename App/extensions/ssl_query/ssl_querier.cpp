#include "ssl_querier.h"
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/pem.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <iostream>


#include <QDebug>

namespace ady{

SSLQuerier::SSLQuerier(const QStringList& list,QObject* parent)
    :QObject(parent),
    NetworkRequest(curl_easy_init()),m_sitelist(list)
{

}

/*static CURLcode ssl_ctx_callback(CURL *curl, void *ssl_ctx, void *user_data) {
    SSL_CTX *ctx = static_cast<SSL_CTX*>(ssl_ctx);
    if (ctx) {
        SSL *ssl = SSL_new(ctx);
        if (ssl) {
            auto ret = SSL_connect(ssl) ;
            qDebug()<<"result"<<ret;
            if (ret == 1) {
                X509 *cert = SSL_get_peer_certificate(ssl);
                if (cert) {
                    BIO *bio = BIO_new_fp(stdout, BIO_NOCLOSE);
                    X509_print(bio, cert);
                    BIO_free(bio);
                    X509_free(cert);
                }
            }
            SSL_free(ssl);
        }
    }
    return CURLE_OK;
}*/



static CURLcode ssl_ctx_callback(CURL *curl, void *ssl_ctx, void *userptr) {
    SSL_CTX *ctx = (SSL_CTX *)ssl_ctx;
    SSL *ssl = SSL_new(ctx); // 创建一个 SSL 对象
    if (ssl) {
        // 获取对等证书
        X509 *cert = SSL_get_peer_certificate(ssl);

        if (cert) {
            qDebug()<<("Certificate information:\n");

            char *subject = X509_NAME_oneline(X509_get_subject_name(cert), NULL, 0);
            qDebug()<<("Subject: %s\n")<<subject;
            OPENSSL_free(subject);

            char *issuer = X509_NAME_oneline(X509_get_issuer_name(cert), NULL, 0);
            qDebug()<<("Issuer: %s\n")<<issuer;
            OPENSSL_free(issuer);

            ASN1_TIME *not_before = X509_get_notBefore(cert);
            ASN1_TIME *not_after = X509_get_notAfter(cert);
            qDebug()<<("Valid from: %s\n")<<not_before->data;
            qDebug()<<("Valid until: %s\n")<<not_after->data;

            X509_free(cert);
        } else {
            qDebug()<<("No certificate found.\n");
        }

        // 释放 SSL 对象
        SSL_free(ssl);
    } else {
        qDebug()<<("Failed to create SSL object.\n");
    }

    return CURLE_OK;
}


bool SSLQuerier::execute(){
    this->setOption(CURLOPT_TIMEOUT,3l);
    this->setOption(CURLOPT_NOBODY,1);
    this->setOption(CURLOPT_HEADER,1);
    //this->setOption(CURLOPT_SSL_CTX_FUNCTION, ssl_ctx_callback);
    this->setOption(CURLOPT_SSL_VERIFYPEER, 0L);
    this->setOption(CURLOPT_SSL_VERIFYHOST, 0L);
    this->setOption(CURLOPT_DOH_SSL_VERIFYSTATUS,0l);


    for(auto one:m_sitelist){
        QString url = "https://"+one;
        this->setOption(CURLOPT_URL,url.toStdString().c_str());
        CURLcode res = curl_easy_perform(this->curl);
        qDebug()<<url<<"code"<<res;
    }
    return true;
}

NetworkResponse* SSLQuerier::customeAccess(const QString& name,QMap<QString,QVariant> data){
    return nullptr;
}

}
