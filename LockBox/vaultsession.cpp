#include "vaultsession.h"

namespace {
    QString g_username;
    QByteArray g_key;
}

void VaultSession::setSession(const QString &username, const QByteArray &key)
{
    g_username = username;
    g_key = key;
}

QString VaultSession::username()
{
    return g_username;
}

QByteArray VaultSession::key()
{
    return g_key;
}

void VaultSession::clear()
{
    g_username.clear();
    g_key.clear();
}
