#include "vault_importer.h"
#include "crypto.h"
#include "database.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QSqlQuery>

bool VaultImporter::importVault(const QString &filePath,
                                const QString &oldUsername,
                                const QString &oldPassword,
                                const QByteArray &newDerivedKey,
                                const QString &newUsername)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonObject root = doc.object();
    if (root["format"].toString() != "LockBoxVaultV1")
        return false;

    if (root["owner"].toString() != oldUsername)
        return false;

    // Fetch old user salt
    QSqlQuery q(Database::getDatabase());
    q.prepare("SELECT salt, verification_ciphertext FROM users WHERE username=?");
    q.addBindValue(oldUsername);

    if (!q.exec() || !q.next())
        return false;

    QByteArray salt = q.value(0).toByteArray();
    QByteArray verify = q.value(1).toByteArray();

    if (!Crypto::verifyMasterPassword(oldPassword, salt, verify))
        return false;

    QByteArray oldKey = Crypto::deriveKey(oldPassword, salt);
    if (oldKey.isEmpty())
        return false;

    QJsonArray vault = root["vault"].toArray();

    for (auto v : vault) {
        QJsonObject e = v.toObject();

        QString site = e["site"].toString();
        QByteArray userCipherOld = QByteArray::fromBase64(e["username"].toString().toUtf8());
        QByteArray passCipherOld = QByteArray::fromBase64(e["password"].toString().toUtf8());

        QString userPlain = Crypto::decrypt(userCipherOld, oldKey);
        QString passPlain = Crypto::decrypt(passCipherOld, oldKey);

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

        Database::addPassword(newUsername, site,
                              userCipherNew, passCipherNew, entryHash);
    }

    return true;
}
