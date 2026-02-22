#pragma once
#include <QByteArray>

QByteArray decryptAESGCM(
    const QByteArray &ciphertext,
    const QByteArray &nonce,
    const QByteArray &key);

QByteArray encryptAESGCM(
    const QByteArray &plaintext,
    QByteArray &iv,
    const QByteArray &key);