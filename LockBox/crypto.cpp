#include "crypto.h"
#include <QDebug>
#include <QByteArray>
#include <QString>
#include <sodium.h>


static const size_t ENCRYPTION_KEY_SIZE = crypto_aead_xchacha20poly1305_IETF_KEYBYTES;

static const size_t KEY_DERIVATION_SALT_SIZE = crypto_pwhash_SALTBYTES;

static const QString VERIFICATION_PHRASE = "MASTER_PASSWORD_VERIFIED";



bool Crypto::initialize() {
    if (sodium_init() == -1) {
        qCritical() << "Libsodium initialization failed! Fatal error.";
        return false;
    }
    return true;
}

// ================= KEY DERIVATION =================

QByteArray Crypto::deriveKey(const QString &masterPassword, const QByteArray &salt) {
    if (salt.size() != KEY_DERIVATION_SALT_SIZE) {
        qCritical() << "Key derivation failed: Invalid salt size.";
        return QByteArray();
    }

    QByteArray password_bytes = masterPassword.toUtf8();
    QByteArray derivedKey(ENCRYPTION_KEY_SIZE, 0);


    int result = crypto_pwhash(
        reinterpret_cast<unsigned char*>(derivedKey.data()), derivedKey.size(),
        password_bytes.constData(), password_bytes.size(),
        reinterpret_cast<const unsigned char*>(salt.constData()),
        crypto_pwhash_OPSLIMIT_MODERATE,
        crypto_pwhash_MEMLIMIT_MODERATE,
        crypto_pwhash_ALG_DEFAULT
        );

    if (result != 0) {
        qCritical() << "Argon2 Key derivation failed!";
        return QByteArray();
    }
    return derivedKey;
}

// ================= ENCRYPTION / DECRYPTION =================

QByteArray Crypto::encrypt(const QString &plaintext, const QByteArray &derivedKey) {
    if (derivedKey.size() != ENCRYPTION_KEY_SIZE) {
        qCritical() << "Encryption failed: Invalid derived key size.";
        return QByteArray();
    }

    QByteArray plaintext_bytes = plaintext.toUtf8();

    QByteArray output_bytes(
        crypto_aead_xchacha20poly1305_IETF_NPUBBYTES +
            plaintext_bytes.size() +
            crypto_aead_xchacha20poly1305_IETF_ABYTES,
        0
        );

    unsigned char nonce[crypto_aead_xchacha20poly1305_IETF_NPUBBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    memcpy(output_bytes.data(), nonce, sizeof(nonce));

    unsigned long long ciphertext_len;
    int result = crypto_aead_xchacha20poly1305_ietf_encrypt(
        reinterpret_cast<unsigned char*>(output_bytes.data()) + crypto_aead_xchacha20poly1305_IETF_NPUBBYTES,
        &ciphertext_len,
        reinterpret_cast<const unsigned char*>(plaintext_bytes.constData()),
        plaintext_bytes.size(),
        nullptr, 0,
        nullptr,
        nonce,
        reinterpret_cast<const unsigned char*>(derivedKey.constData())
        );

    if (result != 0) {
        qCritical() << "XChaCha20-Poly1305 Encryption failed!";
        return QByteArray();
    }

    output_bytes.resize(crypto_aead_xchacha20poly1305_IETF_NPUBBYTES + ciphertext_len);
    return output_bytes;
}

QString Crypto::decrypt(const QByteArray &ciphertext_with_nonce, const QByteArray &derivedKey) {
    if (derivedKey.size() != ENCRYPTION_KEY_SIZE) {
        qCritical() << "Decryption failed: Invalid derived key size.";
        return QString();
    }

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
        reinterpret_cast<const unsigned char*>(derivedKey.constData())
        );

    if (result != 0) {
        qWarning() << "Decryption failed! Wrong key or data was tampered with.";
        return QString();
    }

    plaintext_bytes.resize(plaintext_len);
    return QString::fromUtf8(plaintext_bytes);
}

// ================= MASTER PASSWORD VERIFICATION =================

QByteArray Crypto::registerNewUser(const QString &masterPassword, QByteArray &outSalt, QByteArray &outVerificationCiphertext) {

    outSalt.resize(KEY_DERIVATION_SALT_SIZE);
    randombytes_buf(outSalt.data(), outSalt.size());


    QByteArray derivedKey = deriveKey(masterPassword, outSalt);
    if (derivedKey.isEmpty()) {
        return QByteArray();
    }


    outVerificationCiphertext = encrypt(VERIFICATION_PHRASE, derivedKey);
    if (outVerificationCiphertext.isEmpty()) {
        qCritical() << "Failed to encrypt verification phrase.";
        return QByteArray();
    }

    return derivedKey;
}

bool Crypto::verifyMasterPassword(const QString &masterPassword, const QByteArray &storedSalt, const QByteArray &storedCiphertext) {

    QByteArray derivedKey = deriveKey(masterPassword, storedSalt);
    if (derivedKey.isEmpty()) {
        return false;
    }

    QString decryptedPhrase = decrypt(storedCiphertext, derivedKey);

    return decryptedPhrase == VERIFICATION_PHRASE;
}
