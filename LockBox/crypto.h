#ifndef CRYPTO_H
#define CRYPTO_H

#include <QString>
#include <QByteArray>
#include <sodium.h>

namespace Crypto {

/* ---------- Init ---------- */
bool initialize();

/* ---------- Key Derivation (KEK) ---------- */
QByteArray deriveKey(const QString &masterPassword, const QByteArray &salt);

/* ---------- DEK Handling ---------- */
QByteArray generateDEK();
QByteArray encryptDEK(const QByteArray &dek, const QByteArray &kek);
QByteArray decryptDEK(const QByteArray &encryptedDek, const QByteArray &kek);

/* ---------- Data Encryption (uses DEK) ---------- */
QByteArray encrypt(const QString &plaintext, const QByteArray &dek);
QString decrypt(const QByteArray &ciphertext_with_nonce, const QByteArray &dek);

/* ---------- User Registration / Verification ---------- */
bool verifyMasterPassword(const QString &masterPassword,
                          const QByteArray &storedSalt,
                          const QByteArray &storedVerificationCiphertext);

}

#endif // CRYPTO_H
