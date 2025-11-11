#ifndef CRYPTO_H
#define CRYPTO_H

#include <QString>
#include <QByteArray>
#include <sodium.h>

namespace Crypto {


bool initialize();

QByteArray deriveKey(const QString &masterPassword, const QByteArray &salt);

QByteArray registerNewUser(const QString &masterPassword, QByteArray &outSalt, QByteArray &outVerificationCiphertext);

QByteArray encrypt(const QString &plaintext, const QByteArray &derivedKey);
QString decrypt(const QByteArray &ciphertext_with_nonce, const QByteArray &derivedKey);

bool verifyMasterPassword(const QString &masterPassword, const QByteArray &storedSalt, const QByteArray &storedCiphertext);

}

#endif
