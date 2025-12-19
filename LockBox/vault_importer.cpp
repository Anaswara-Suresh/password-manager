#include "vault_importer.h"
#include "crypto.h"
#include "database.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlQuery>

#include <sodium.h>

bool VaultImporter::importVault(const QString &filePath,
                                const QString &oldUsername,
                                const QString &oldPassword,
                                const QByteArray &newDerivedKey,
                                const QString &newUsername)
{
    /* ---------- Load export file ---------- */
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject())
        return false;

    QJsonObject root = doc.object();

    /* ---------- Validate format ---------- */
    if (root["format"].toString() != "LockBoxVaultV1")
        return false;

    if (root["owner"].toString() != oldUsername)
        return false;

    /* ---------- Resolve salt + verification ---------- */
    QByteArray salt;
    QByteArray verification;
    bool userExistsLocally = false;

    QSqlQuery q(Database::getDatabase());
    q.prepare("SELECT salt, verification_ciphertext FROM users WHERE username=?");
    q.addBindValue(oldUsername);

    if (q.exec() && q.next()) {
        salt = q.value(0).toByteArray();
        verification = q.value(1).toByteArray();
        userExistsLocally = true;
    }

    // Fallback: use export file (cross-device import)
    if (!userExistsLocally) {
        if (!root.contains("salt") || !root.contains("verification"))
            return false;

        salt = QByteArray::fromBase64(root["salt"].toString().toUtf8());
        verification = QByteArray::fromBase64(root["verification"].toString().toUtf8());
    }

    /* ---------- Verify old password ---------- */
    if (!Crypto::verifyMasterPassword(oldPassword, salt, verification))
        return false;

    QByteArray oldKey = Crypto::deriveKey(oldPassword, salt);
    if (oldKey.isEmpty())
        return false;

    /* ---------- Import entries ---------- */
    QJsonArray vault = root["vault"].toArray();
    if (vault.isEmpty())
        return true;

    Database::beginTransaction();

    for (const QJsonValue &v : vault) {
        QJsonObject e = v.toObject();

        QString site = e["site"].toString();
        QByteArray userCipherOld =
            QByteArray::fromBase64(e["username"].toString().toUtf8());
        QByteArray passCipherOld =
            QByteArray::fromBase64(e["password"].toString().toUtf8());

        QString userPlain = Crypto::decrypt(userCipherOld, oldKey);
        QString passPlain = Crypto::decrypt(passCipherOld, oldKey);

        if (userPlain.isEmpty() || passPlain.isEmpty()) {
            Database::rollback();
            return false;
        }

        QByteArray userCipherNew = Crypto::encrypt(userPlain, newDerivedKey);
        QByteArray passCipherNew = Crypto::encrypt(passPlain, newDerivedKey);


        QString combined = site + ":" + userPlain;
        QByteArray entryHash(crypto_generichash_BYTES, 0);

        crypto_generichash(
            reinterpret_cast<unsigned char*>(entryHash.data()),
            entryHash.size(),
            reinterpret_cast<const unsigned char*>(combined.toUtf8().data()),
            combined.size(),
            nullptr, 0
            );

        Database::addPassword(
            newUsername,
            site,
            userCipherNew,
            passCipherNew,
            entryHash
            );


        userPlain.fill('\0');
        passPlain.fill('\0');
    }

    Database::commit();
    return true;
}
