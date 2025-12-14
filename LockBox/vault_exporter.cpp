#include "vault_exporter.h"
#include "database.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QSqlQuery>

bool VaultExporter::exportVault(const QString &username,
                                const QByteArray &derivedKey,
                                const QString &filePath)
{
    QList<QVariantMap> vault = Database::getFullVault(username);
    if (vault.isEmpty())
        return false;

    QJsonArray entries;

    for (auto &row : vault) {
        QJsonObject obj;
        obj["site"] = QString::fromUtf8(row["site"].toByteArray());
        obj["username"] = QString(row["username"].toByteArray().toBase64());
        obj["password"] = QString(row["password"].toByteArray().toBase64());
        entries.append(obj);
    }

    QJsonObject root;
    root["format"] = "LockBoxVaultV1";
    root["owner"] = username;
    root["vault"] = entries;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();

    return true;
}
