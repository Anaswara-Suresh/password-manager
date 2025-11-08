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

QByteArray Crypto::encrypt(const QString &plaintext) {
    QByteArray plaintext_bytes = plaintext.toUtf8();

    // Allocate space for Nonce + Ciphertext + Authentication Tag
    QByteArray output_bytes(
        crypto_aead_xchacha20poly1305_IETF_NPUBBYTES + plaintext_bytes.size() + crypto_aead_xchacha20poly1305_IETF_ABYTES,
        0
        );

    // 1. Generate a unique Nonce (used once)
    unsigned char nonce[crypto_aead_xchacha20poly1305_IETF_NPUBBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    // Copy Nonce to the start of the output buffer
    memcpy(output_bytes.data(), nonce, sizeof(nonce));

    // 2. Encrypt
    unsigned long long ciphertext_len;
    int result = crypto_aead_xchacha20poly1305_ietf_encrypt(
        (unsigned char*)output_bytes.data() + crypto_aead_xchacha20poly1305_IETF_NPUBBYTES, // Output buffer, offset after nonce
        &ciphertext_len,
        (const unsigned char*)plaintext_bytes.constData(),
        (unsigned long long)plaintext_bytes.size(),
        NULL, 0, // No additional data (ad and adlen)
        NULL, // Existing secret nonce (we generated a new one)
        nonce,
        ENCRYPTION_KEY
        );

    if (result != 0) {
        qCritical() << "Encryption failed!";
        return QByteArray();
    }

    // Resize QByteArray to actual size: Nonce size + Ciphertext size (which includes the tag)
    output_bytes.resize(crypto_aead_xchacha20poly1305_IETF_NPUBBYTES + ciphertext_len);
    return output_bytes;
}

QString Crypto::decrypt(const QByteArray &ciphertext_with_nonce) {
    if (ciphertext_with_nonce.size() < crypto_aead_xchacha20poly1305_IETF_NPUBBYTES + crypto_aead_xchacha20poly1305_IETF_ABYTES) {
        qCritical() << "Ciphertext too short to contain nonce and tag!";
        return QString();
    }

    // 1. Extract Nonce and Ciphertext
    const unsigned char* nonce = (const unsigned char*)ciphertext_with_nonce.constData();
    const unsigned char* ciphertext = (const unsigned char*)ciphertext_with_nonce.constData() + crypto_aead_xchacha20poly1305_IETF_NPUBBYTES;
    int ciphertext_len = ciphertext_with_nonce.size() - crypto_aead_xchacha20poly1305_IETF_NPUBBYTES;

    QByteArray plaintext_bytes(ciphertext_len, 0);

    // 2. Decrypt
    unsigned long long plaintext_len;
    int result = crypto_aead_xchacha20poly1305_ietf_decrypt(
        (unsigned char*)plaintext_bytes.data(),
        &plaintext_len,
        NULL, // Additional data pointer
        ciphertext,
        (unsigned long long)ciphertext_len, // Ciphertext length
        NULL, 0, // Additional data pointer and length (none used)
        nonce,
        ENCRYPTION_KEY
        );

    if (result != 0) {
        qCritical() << "Decryption failed! Data may be tampered with or key is wrong.";
        return QString();
    }

    // Resize QByteArray to actual plaintext size
    plaintext_bytes.resize(plaintext_len);

    return QString::fromUtf8(plaintext_bytes);
}
