#ifndef CRYPTO_H
#define CRYPTO_H

#include <QString>
#include <QByteArray>
#include <sodium.h>

namespace Crypto {


bool initialize();


QByteArray encrypt(const QString &plaintext);


QString decrypt(const QByteArray &ciphertext_with_nonce);

}

#endif
