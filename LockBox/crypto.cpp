#include "crypto.h"
#include <QDebug>

namespace Crypto {

bool initialize() {
    if (sodium_init() < 0) {
        return false;
    }
    return true;
}

/* ---------- KEK ---------- */
QByteArray deriveKey(const QString &masterPassword, const QByteArray &salt) {
    QByteArray key(crypto_aead_xchacha20poly1305_ietf_KEYBYTES, 0);

    crypto_pwhash(
        reinterpret_cast<unsigned char *>(key.data()),
        key.size(),
        masterPassword.toUtf8().constData(),
        masterPassword.size(),
        reinterpret_cast<const unsigned char *>(salt.constData()),
        crypto_pwhash_OPSLIMIT_INTERACTIVE,
        crypto_pwhash_MEMLIMIT_INTERACTIVE,
        crypto_pwhash_ALG_DEFAULT
        );

    return key;
}

/* ---------- DEK ---------- */
QByteArray generateDEK() {
    QByteArray dek(crypto_aead_xchacha20poly1305_ietf_KEYBYTES, 0);
    randombytes_buf(dek.data(), dek.size());
    return dek;
}

QByteArray encryptDEK(const QByteArray &dek, const QByteArray &kek) {
    return encrypt(QString::fromUtf8(dek.toBase64()), kek);
}

QByteArray decryptDEK(const QByteArray &encryptedDek, const QByteArray &kek) {
    QString base64 = decrypt(encryptedDek, kek);
    return QByteArray::fromBase64(base64.toUtf8());
}

/* ---------- Data Encryption ---------- */
QByteArray encrypt(const QString &plaintext, const QByteArray &key) {
    QByteArray nonce(crypto_aead_xchacha20poly1305_ietf_NPUBBYTES, 0);
    randombytes_buf(nonce.data(), nonce.size());

    QByteArray ciphertext(
        plaintext.size() + crypto_aead_xchacha20poly1305_ietf_ABYTES, 0
        );

    unsigned long long cipherLen;

    crypto_aead_xchacha20poly1305_ietf_encrypt(
        reinterpret_cast<unsigned char *>(ciphertext.data()),
        &cipherLen,
        reinterpret_cast<const unsigned char *>(plaintext.toUtf8().constData()),
        plaintext.size(),
        nullptr,
        0,
        nullptr,
        reinterpret_cast<unsigned char *>(nonce.data()),
        reinterpret_cast<const unsigned char *>(key.constData())
        );

    ciphertext.resize(cipherLen);
    return nonce + ciphertext;
}

QString decrypt(const QByteArray &ciphertext_with_nonce, const QByteArray &key) {
    QByteArray nonce = ciphertext_with_nonce.left(
        crypto_aead_xchacha20poly1305_ietf_NPUBBYTES
        );
    QByteArray ciphertext = ciphertext_with_nonce.mid(nonce.size());

    QByteArray decrypted(ciphertext.size(), 0);
    unsigned long long decryptedLen;

    if (crypto_aead_xchacha20poly1305_ietf_decrypt(
            reinterpret_cast<unsigned char *>(decrypted.data()),
            &decryptedLen,
            nullptr,
            reinterpret_cast<const unsigned char *>(ciphertext.constData()),
            ciphertext.size(),
            nullptr,
            0,
            reinterpret_cast<const unsigned char *>(nonce.constData()),
            reinterpret_cast<const unsigned char *>(key.constData())
            ) != 0) {
        return QString();
    }

    decrypted.resize(decryptedLen);
    return QString::fromUtf8(decrypted);
}

/* ---------- Verification ---------- */
bool verifyMasterPassword(const QString &masterPassword,
                          const QByteArray &storedSalt,
                          const QByteArray &storedVerificationCiphertext) {
    QByteArray kek = deriveKey(masterPassword, storedSalt);
    QString result = decrypt(storedVerificationCiphertext, kek);
    kek.fill(0);
    return result == "MASTER_PASSWORD_VERIFIED";
}

}
