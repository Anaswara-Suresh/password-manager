#include "totp.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QtMath>

static QByteArray base32Decode(const QString &input)
{
    static const QString alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

    QString cleaned = input.toUpper();
    cleaned.remove(' ');
    cleaned.remove('=');

    QByteArray output;
    int buffer = 0;
    int bitsLeft = 0;

    for (QChar c : cleaned) {
        int val = alphabet.indexOf(c);
        if (val < 0)
            continue; 

        buffer = (buffer << 5) | val;
        bitsLeft += 5;

        if (bitsLeft >= 8) {
            output.append(char((buffer >> (bitsLeft - 8)) & 0xFF));
            bitsLeft -= 8;
        }
    }

    return output;
}


static QByteArray hmacSha1(const QByteArray &key, const QByteArray &msg)
{
    const int blockSize = 64;
    QByteArray k = key;

    if (k.size() > blockSize)
        k = QCryptographicHash::hash(k, QCryptographicHash::Sha1);

    k = k.leftJustified(blockSize, 0x00);

    QByteArray oKeyPad(blockSize, 0x5c);
    QByteArray iKeyPad(blockSize, 0x36);

    for (int i = 0; i < blockSize; ++i) {
        oKeyPad[i] ^= k[i];
        iKeyPad[i] ^= k[i];
    }

    QByteArray inner =
        QCryptographicHash::hash(iKeyPad + msg, QCryptographicHash::Sha1);

    return QCryptographicHash::hash(oKeyPad + inner,
                                    QCryptographicHash::Sha1);
}


QString TOTP::generate(const QString &base32Secret,
                       int digits,
                       int period)
{
    QByteArray secret = base32Decode(base32Secret);

    qint64 unixTime = QDateTime::currentSecsSinceEpoch();
    qint64 counter = unixTime / period;

    QByteArray counterBytes(8, 0x00);
    for (int i = 7; i >= 0; --i) {
        counterBytes[i] = counter & 0xFF;
        counter >>= 8;
    }

    QByteArray hash = hmacSha1(secret, counterBytes);

    int offset = hash.at(hash.size() - 1) & 0x0F;

    int binary =
        ((hash[offset] & 0x7F) << 24) |
        ((hash[offset + 1] & 0xFF) << 16) |
        ((hash[offset + 2] & 0xFF) << 8) |
        (hash[offset + 3] & 0xFF);

    int otp = binary % static_cast<int>(qPow(10, digits));

    return QString("%1").arg(otp, digits, 10, QChar('0'));
}
