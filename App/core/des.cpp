#include "des.h"
#include <openssl/des.h>
#include <openssl/pkcs7.h>
#include <string>


#include <QDebug>

#ifndef uchar
#define uchar unsigned char
#endif

namespace ady {
QString DES::key = "PQt4jzZ9";
QString DES::vt = "huUljjRv";



QString DES::encode(const QString& str,const QString& key,const QString& vt){
    DES_cblock k;
    DES_cblock vtKey;
    DES_key_schedule ks;


    DES_string_to_key(vt.toStdString().c_str(), &vtKey);
    DES_string_to_key(key.toStdString().c_str(), &k);


    DES_set_key_unchecked(&k, &ks);

    QByteArray by;
    std::string tmp = str.toStdString();
    int length = tmp.length();


    int padding = (8 - length % 8) % 8;
    tmp.append(padding, '\0');

    unsigned char encrypted_buffer[256] = {0};

    DES_cbc_encrypt(reinterpret_cast<const unsigned char *>(tmp.c_str()),
                    encrypted_buffer,
                    tmp.length() + padding, &ks, &vtKey, DES_ENCRYPT);

    by.append(reinterpret_cast<const char*>(encrypted_buffer), tmp.length() + padding);

    return by.toBase64();
}
QString DES::decode(const QString& str,const QString& key,const QString& vt){
    DES_cblock k = {0};
    DES_cblock vtKey = {0};
    DES_key_schedule ks;

    unsigned char encrypted_buffer[256] = {0};
    unsigned char decrypted_buffer[256] = {0};

    DES_string_to_key(vt.toStdString().c_str(), &vtKey);
    DES_string_to_key(key.toStdString().c_str(), &k);

    DES_set_key_unchecked(&k, &ks);

    QByteArray by = QByteArray::fromBase64(str.toUtf8());

    int length = by.size();
    memcpy(encrypted_buffer, by.data(), length);

    DES_cbc_encrypt(encrypted_buffer, decrypted_buffer, length, &ks, &vtKey, DES_DECRYPT);

    std::string decrypted(reinterpret_cast<char*>(decrypted_buffer), length);

    size_t pos = decrypted.find_first_of('\0');
    if (pos != std::string::npos) {
        decrypted.erase(pos);
    }
    return QString::fromStdString(decrypted);
}

QString DES::encode(const QString& str,const QString &key1)
{
    return DES::encode(str,key1,DES::vt);
}

QString DES::decode(const QString& str,const QString &key1)
{

    return DES::decode(str,key1,DES::vt);
}

QString DES::encode(const QString& str)
{
    return encode(str,key,DES::vt);
}

QString DES::decode(const QString& str)
{
    return decode(str,key,DES::vt);
}

}
