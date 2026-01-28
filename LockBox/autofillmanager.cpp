#include "autofillmanager.h"
#include "vaultsession.h"
#include "database.h"
#include "crypto.h"
#include <QUrl>
#include <QJsonObject>
#include <QJsonArray>

static QString extractDomain(const QString &url)
{
    QUrl qurl(url);
    QString host = qurl.host();    // github.com
    if (host.isEmpty())
        host = url;                // fallback if DB stored raw site string
    return host;
}

QJsonArray AutofillManager::getCredentialsForUrl(const QString &url)
{
    QJsonArray arr;

    QString username = VaultSession::username();
    QByteArray key = VaultSession::key();

    if (username.isEmpty() || key.isEmpty()) {
        return arr;
    }

    QString domain = extractDomain(url);

    // 1) Query DB records that match domain
    QList<QVariantMap> rows = Database::findBySite(username, domain);

    // 2) Decrypt and convert to JSON
    for (auto &row : rows) {
        QString decUser = Crypto::decrypt(row["username"].toByteArray(), key);
        QString decPass = Crypto::decrypt(row["password"].toByteArray(), key);

        if (decUser.isEmpty() && !row["username"].toByteArray().isEmpty())
            continue;

        QJsonObject entry;
        entry["site"] = row["site"].toString();
        entry["username"] = decUser;
        entry["password"] = decPass;
        entry["url"] = url;

        arr.append(entry);
    }

    return arr;
}
