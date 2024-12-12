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
    char encrypted_buffer[256];
    DES_cblock k;
    DES_cblock vtKey;
    DES_key_schedule ks;
    DES_string_to_key(vt.toStdString().c_str(), &vtKey);
    DES_string_to_key(key.toStdString().c_str(), &k);
    DES_set_key_unchecked((const_DES_cblock *)&k, &ks);
    memset(encrypted_buffer, 0, sizeof(encrypted_buffer));
    QByteArray by;
    int length = str.length();
    std::string tmp;
    tmp.assign(str.toStdString());
    int padding = 0;
    if(length % 8 > 0){
        padding = 8 - (length / 8);
        for(int i=0;i<padding;i++){
            tmp.append("",1);
        }
    }

    DES_cbc_encrypt(reinterpret_cast<const unsigned char *>(tmp.c_str()),
                    reinterpret_cast<unsigned char *>(encrypted_buffer),
                    length + padding, &ks, &vtKey, DES_ENCRYPT);
    by.append((const char *)encrypted_buffer,tmp.length());
    return by.toBase64();
}
QString DES::decode(const QString& str,const QString& key,const QString& vt){
    DES_cblock       k = {0};
    DES_cblock       vtKey = {0};
    DES_key_schedule ks  ;

    unsigned char encrypted_buffer[256] = {0};
    unsigned char decrypted_buffer[256] = {0};

    DES_string_to_key(vt.toStdString().c_str(), &vtKey);
    DES_string_to_key(key.toStdString().c_str(), &k);

    DES_set_key_unchecked(&k, &ks);

    std::string decrypted;

    memset(encrypted_buffer, 0, sizeof(encrypted_buffer));
    memset(decrypted_buffer, 0, sizeof(decrypted_buffer));

    QByteArray by = QByteArray::fromBase64(str.toLocal8Bit());

    const char* string = by.data();
    int length = strlen(string);

    memcpy(encrypted_buffer, string, length);

    DES_cbc_encrypt(encrypted_buffer, decrypted_buffer, length,
                    &ks, &vtKey, DES_DECRYPT);

    decrypted_buffer[length] = 0;

    decrypted.assign(reinterpret_cast<char*>(decrypted_buffer));

    /*size_t pos = decrypted.find_first_of(" ");
      if (pos != std::string::npos) {
          decrypted.erase(pos);
      }*/

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
