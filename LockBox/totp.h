#ifndef TOTP_H
#define TOTP_H

#include <QString>
#include <QByteArray>

class TOTP
{
public:
    static QString generate(const QString &base32Secret,
                            int digits = 6,
                            int period = 30);
};

#endif
