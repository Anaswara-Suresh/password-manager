#include "aesgcm.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

QByteArray decryptAESGCM(
    const QByteArray &ciphertextWithTag,
    const QByteArray &iv,
    const QByteArray &key)
{
    if (ciphertextWithTag.size() < 16)
        return QByteArray();

    QByteArray tag = ciphertextWithTag.right(16);
    QByteArray ciphertext =
        ciphertextWithTag.left(ciphertextWithTag.size() - 16);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        return QByteArray();

    QByteArray plaintext(ciphertext.size(), 0);
    int len = 0;
    int plaintext_len = 0;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1)
        return QByteArray();

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv.size(), NULL) != 1)
        return QByteArray();

    if (EVP_DecryptInit_ex(ctx, NULL, NULL,
                           reinterpret_cast<const unsigned char*>(key.constData()),
                           reinterpret_cast<const unsigned char*>(iv.constData())) != 1)
        return QByteArray();

    if (EVP_DecryptUpdate(ctx,
                          reinterpret_cast<unsigned char*>(plaintext.data()),
                          &len,
                          reinterpret_cast<const unsigned char*>(ciphertext.constData()),
                          ciphertext.size()) != 1)
        return QByteArray();

    plaintext_len = len;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG,
                            16,
                            const_cast<unsigned char*>(
                                reinterpret_cast<const unsigned char*>(tag.constData()))) != 1)
        return QByteArray();

    if (EVP_DecryptFinal_ex(ctx,
                            reinterpret_cast<unsigned char*>(plaintext.data()) + len,
                            &len) <= 0)
        return QByteArray();

    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    plaintext.resize(plaintext_len);
    return plaintext;
}

QByteArray encryptAESGCM(
    const QByteArray &plaintext,
    QByteArray &iv,
    const QByteArray &key)
{
    iv.resize(12);
    RAND_bytes(reinterpret_cast<unsigned char*>(iv.data()), iv.size());

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        return QByteArray();

    QByteArray ciphertext(plaintext.size() + 16, 0);
    int len = 0;
    int ciphertext_len = 0;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1)
        return QByteArray();

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv.size(), NULL) != 1)
        return QByteArray();

    if (EVP_EncryptInit_ex(ctx, NULL, NULL,
                           reinterpret_cast<const unsigned char*>(key.constData()),
                           reinterpret_cast<const unsigned char*>(iv.constData())) != 1)
        return QByteArray();

    if (EVP_EncryptUpdate(ctx,
                          reinterpret_cast<unsigned char*>(ciphertext.data()),
                          &len,
                          reinterpret_cast<const unsigned char*>(plaintext.constData()),
                          plaintext.size()) != 1)
        return QByteArray();

    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx,
                            reinterpret_cast<unsigned char*>(ciphertext.data()) + len,
                            &len) != 1)
        return QByteArray();

    ciphertext_len += len;

    unsigned char tag[16];
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag) != 1)
        return QByteArray();

    EVP_CIPHER_CTX_free(ctx);

    QByteArray final = ciphertext.left(ciphertext_len);
    final.append(reinterpret_cast<char*>(tag), 16);

    return final;
}