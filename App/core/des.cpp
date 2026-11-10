#include "des.h"

#include <mbedtls/des.h>

#include <QByteArray>
#include <cstring>

namespace ady {

namespace {

constexpr int BLOCK_SIZE = 8;

// 等价于 OpenSSL 的 DES_set_odd_parity：修正每个字节的奇偶校验位
void setOddParity(unsigned char key[BLOCK_SIZE])
{
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        unsigned char value = key[i] & 0xfe;
        int ones = 0;
        for (int bit = 1; bit < 8; ++bit) {
            ones += (value >> bit) & 0x01;
        }
        if ((ones % 2) == 0) {
            value |= 0x01;
        }
        key[i] = value;
    }
}

// 等价于 OpenSSL 的 DES_cbc_cksum：CBC 加密（末尾不足一个分组时补零），输出最后一个密文分组
void cbcCksum(const unsigned char* in, int length, const unsigned char key[BLOCK_SIZE], unsigned char out[BLOCK_SIZE])
{
    mbedtls_des_context ctx;
    mbedtls_des_init(&ctx);
    mbedtls_des_setkey_enc(&ctx, key);

    unsigned char chain[BLOCK_SIZE];
    std::memcpy(chain, key, BLOCK_SIZE);

    int offset = 0;
    while (offset < length) {
        const int count = (length - offset) >= BLOCK_SIZE ? BLOCK_SIZE : (length - offset);
        unsigned char block[BLOCK_SIZE] = {0};
        std::memcpy(block, in + offset, count);
        for (int i = 0; i < BLOCK_SIZE; ++i) {
            block[i] ^= chain[i];
        }
        mbedtls_des_crypt_ecb(&ctx, block, chain);
        offset += count;
    }

    std::memcpy(out, chain, BLOCK_SIZE);
    mbedtls_des_free(&ctx);
}

// 等价于 OpenSSL 的 DES_string_to_key，保证与历史加密数据兼容
void stringToKey(const QByteArray& str, unsigned char key[BLOCK_SIZE])
{
    std::memset(key, 0, BLOCK_SIZE);
    const int length = str.size();
    for (int i = 0; i < length; ++i) {
        unsigned char j = static_cast<unsigned char>(str.at(i));
        if ((i % 16) < 8) {
            key[i % 8] ^= static_cast<unsigned char>(j << 1);
        } else {
            // 反转比特序
            j = static_cast<unsigned char>(((j << 4) & 0xf0) | ((j >> 4) & 0x0f));
            j = static_cast<unsigned char>(((j << 2) & 0xcc) | ((j >> 2) & 0x33));
            j = static_cast<unsigned char>(((j << 1) & 0xaa) | ((j >> 1) & 0x55));
            key[7 - (i % 8)] ^= j;
        }
    }
    setOddParity(key);
    cbcCksum(reinterpret_cast<const unsigned char*>(str.constData()), length, key, key);
    setOddParity(key);
}

}

QString DES::key = "PQt4jzZ9";
QString DES::vt = "huUljjRv";

QString DES::encode(const QString& str,const QString& key,const QString& vt){
    unsigned char desKey[BLOCK_SIZE] = {0};
    unsigned char iv[BLOCK_SIZE] = {0};
    stringToKey(key.toUtf8(), desKey);
    stringToKey(vt.toUtf8(), iv);

    const QByteArray plain = str.toUtf8();
    const int length = plain.size();
    const int padding = (BLOCK_SIZE - length % BLOCK_SIZE) % BLOCK_SIZE;
    const int full_length = length + padding;

    QByteArray data(full_length, '\0');
    std::memcpy(data.data(), plain.constData(), length);

    mbedtls_des_context ctx;
    mbedtls_des_init(&ctx);
    mbedtls_des_setkey_enc(&ctx, desKey);
    mbedtls_des_crypt_cbc(&ctx, MBEDTLS_DES_ENCRYPT, full_length,
                          iv,
                          reinterpret_cast<const unsigned char*>(data.constData()),
                          reinterpret_cast<unsigned char*>(data.data()));

    QByteArray by = data;
    if (padding > 0) {
        // 与 OpenSSL DES_cbc_encrypt 处理末尾不足一个分组的方式保持一致：
        // 末尾补零后再整体加密，只保留前 padding 个字节
        unsigned char last[BLOCK_SIZE] = {0};
        unsigned char tail[BLOCK_SIZE] = {0};
        std::memcpy(last, data.constData() + full_length - BLOCK_SIZE, BLOCK_SIZE);
        mbedtls_des_crypt_ecb(&ctx, last, tail);
        by.append(reinterpret_cast<const char*>(tail), padding);
    }
    mbedtls_des_free(&ctx);

    return by.toBase64();
}

QString DES::decode(const QString& str,const QString& key,const QString& vt){
    unsigned char desKey[BLOCK_SIZE] = {0};
    unsigned char iv[BLOCK_SIZE] = {0};
    stringToKey(key.toUtf8(), desKey);
    stringToKey(vt.toUtf8(), iv);

    const QByteArray by = QByteArray::fromBase64(str.toUtf8());
    // 末尾不足一个分组的字节是加密时补零产生的，不参与解密
    const int length = by.size() / BLOCK_SIZE * BLOCK_SIZE;
    if (length == 0) {
        return QString();
    }

    QByteArray data(length, '\0');
    std::memcpy(data.data(), by.constData(), length);

    mbedtls_des_context ctx;
    mbedtls_des_init(&ctx);
    mbedtls_des_setkey_dec(&ctx, desKey);
    mbedtls_des_crypt_cbc(&ctx, MBEDTLS_DES_DECRYPT, length,
                          iv,
                          reinterpret_cast<const unsigned char*>(data.constData()),
                          reinterpret_cast<unsigned char*>(data.data()));
    mbedtls_des_free(&ctx);

    int pos = data.indexOf('\0');
    if (pos < 0) {
        pos = data.size();
    }
    return QString::fromUtf8(data.constData(), pos);
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
