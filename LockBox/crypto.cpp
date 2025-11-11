#include "crypto.h"
#include <QDebug>
#include <QByteArray>
#include <QString>
#include <sodium.h>

// Hardcoded 32-byte key. MUST be exactly 32 characters long.
// In a production app, this key MUST be derived from the user's master password.
static const unsigned char ENCRYPTION_KEY[crypto_aead_xchacha20poly1305_IETF_KEYBYTES] =
    "A-Very-Secret-Key-1234567890123"; // 32 characters

bool Crypto::initialize() {
    if (sodium_init() == -1) {
        qCritical() << "Libsodium initialization failed!";
        return false;
    }
    return true;
}

// ================= ENCRYPTION =================

QByteArray Crypto::encrypt(const QString &plaintext) {
    QByteArray plaintext_bytes = plaintext.toUtf8();

    // Allocate space for Nonce + Ciphertext + Authentication Tag
    QByteArray output_bytes(
        crypto_aead_xchacha20poly1305_IETF_NPUBBYTES +
        plaintext_bytes.size() +
        crypto_aead_xchacha20poly1305_IETF_ABYTES,
        0
    );

    // Generate Nonce
    unsigned char nonce[crypto_aead_xchacha20poly1305_IETF_NPUBBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    // Copy Nonce to start of output
    memcpy(output_bytes.data(), nonce, sizeof(nonce));

    // Encrypt
    unsigned long long ciphertext_len;
    int result = crypto_aead_xchacha20poly1305_ietf_encrypt(
        reinterpret_cast<unsigned char*>(output_bytes.data()) + crypto_aead_xchacha20poly1305_IETF_NPUBBYTES,
        &ciphertext_len,
        reinterpret_cast<const unsigned char*>(plaintext_bytes.constData()),
        plaintext_bytes.size(),
        nullptr, 0,
        nullptr,
        nonce,
        ENCRYPTION_KEY
    );

    if (result != 0) {
        qCritical() << "Encryption failed!";
        return QByteArray();
    }

    output_bytes.resize(crypto_aead_xchacha20poly1305_IETF_NPUBBYTES + ciphertext_len);
    return output_bytes;
}

QString Crypto::decrypt(const QByteArray &ciphertext_with_nonce) {
    if (ciphertext_with_nonce.size() < crypto_aead_xchacha20poly1305_IETF_NPUBBYTES + crypto_aead_xchacha20poly1305_IETF_ABYTES) {
        qCritical() << "Ciphertext too short to contain nonce and tag!";
        return QString();
    }

    const unsigned char* nonce = reinterpret_cast<const unsigned char*>(ciphertext_with_nonce.constData());
    const unsigned char* ciphertext = reinterpret_cast<const unsigned char*>(ciphertext_with_nonce.constData()) + crypto_aead_xchacha20poly1305_IETF_NPUBBYTES;
    int ciphertext_len = ciphertext_with_nonce.size() - crypto_aead_xchacha20poly1305_IETF_NPUBBYTES;

    QByteArray plaintext_bytes(ciphertext_len, 0);

    unsigned long long plaintext_len;
    int result = crypto_aead_xchacha20poly1305_ietf_decrypt(
        reinterpret_cast<unsigned char*>(plaintext_bytes.data()),
        &plaintext_len,
        nullptr,
        ciphertext,
        ciphertext_len,
        nullptr, 0,
        nonce,
        ENCRYPTION_KEY
    );

    if (result != 0) {
        qCritical() << "Decryption failed! Data may be tampered with or key is wrong.";
        return QString();
    }

    plaintext_bytes.resize(plaintext_len);
    return QString::fromUtf8(plaintext_bytes);
}

// ================= PASSWORD HASHING =================

QByteArray Crypto::hashPassword(const QString &password) {
    QByteArray password_bytes = password.toUtf8();
    QByteArray hash(crypto_pwhash_STRBYTES, 0);

    int result = crypto_pwhash_str(
        hash.data(),
        password_bytes.constData(),
        static_cast<unsigned long long>(password_bytes.size()),
        crypto_pwhash_OPSLIMIT_INTERACTIVE,
        crypto_pwhash_MEMLIMIT_INTERACTIVE
    );

    if (result != 0) {
        qCritical() << "Password hashing failed!";
        return QByteArray();
    }

    return hash;
}

bool Crypto::verifyPassword(const QString &password, const QByteArray &storedHash) {
    QByteArray password_bytes = password.toUtf8();

    int result = crypto_pwhash_str_verify(
        storedHash.constData(),
        password_bytes.constData(),
        static_cast<unsigned long long>(password_bytes.size())
    );

    return result == 0; // 0 means match
}
