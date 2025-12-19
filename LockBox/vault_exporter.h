#ifndef VAULT_EXPORTER_H
#define VAULT_EXPORTER_H

#include <QString>
#include <QByteArray>

class VaultExporter {
public:
    static bool exportVault(const QString &username,
                            const QByteArray &derivedKey,
                            const QString &filePath);
};

#endif
