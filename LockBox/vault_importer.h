#ifndef VAULT_IMPORTER_H
#define VAULT_IMPORTER_H

#include <QString>
#include <QByteArray>

class VaultImporter {
public:
    static bool importVault(const QString &filePath,
                            const QString &oldUsername,
                            const QString &oldPassword,
                            const QByteArray &newDerivedKey,
                            const QString &newUsername);
};

#endif
