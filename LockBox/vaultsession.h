#pragma once
#include <QString>
#include <QByteArray>

class VaultSession
{
public:
    static void setSession(const QString &username, const QByteArray &key);
    static QString username();
    static QByteArray key();
    static void clear();
};
