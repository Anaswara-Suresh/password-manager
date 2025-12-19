#include "vault_exporter.h"
#include "database.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QSqlQuery>
#include <QVariant>

bool VaultExporter::exportVault(const QString &username,
                                const QByteArray &derivedKey,
                                const QString &filePath)
{
    Q_UNUSED(derivedKey);

    // ---- Fetch vault entries ----
    QList<QVariantMap> vault = Database::getFullVault(username);
    if (vault.isEmpty())
        return false;

    QJsonArray entries;
    for (const auto &row : vault) {
        QJsonObject obj;
        obj["site"]     = row["site"].toString();
        obj["username"] = QString::fromUtf8(row["username"].toByteArray().toBase64());
        obj["password"] = QString::fromUtf8(row["password"].toByteArray().toBase64());
        entries.append(obj);
    }

    // ---- Fetch user salt + verification ----
    QSqlQuery q(Database::getDatabase());
    q.prepare("SELECT salt, verification_ciphertext FROM users WHERE username=?");
    q.addBindValue(username);

    if (!q.exec() || !q.next())
        return false;

    QByteArray salt   = q.value(0).toByteArray();
    QByteArray verify = q.value(1).toByteArray();


    QJsonObject root;
    root["format"]       = "LockBoxVaultV1";
    root["owner"]        = username;
    root["salt"]         = QString::fromUtf8(salt.toBase64());
    root["verification"] = QString::fromUtf8(verify.toBase64());
    root["vault"]        = entries;


    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();

    return true;
}
