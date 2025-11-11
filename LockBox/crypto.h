#ifndef CRYPTO_H
#define CRYPTO_H

#include <QString>
#include <QByteArray>
#include <sodium.h>

namespace Crypto {

// ===== Initialization =====
bool initialize();

// ===== Encryption for vault data =====
QByteArray encrypt(const QString &plaintext);
QString decrypt(const QByteArray &ciphertext_with_nonce);

// ===== Password hashing for login system =====
QByteArray hashPassword(const QString &password);
bool verifyPassword(const QString &password, const QByteArray &storedHash);

}

#endif // CRYPTO_H
