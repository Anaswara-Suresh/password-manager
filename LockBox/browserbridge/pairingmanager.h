#ifndef PAIRINGMANAGER_H
#define PAIRINGMANAGER_H

#include <QMap>
#include <QByteArray>
#include <QJsonObject>

class PairingManager
{
public:
    static QJsonObject handlePairRequest();

    static bool isClientTrusted(const QString &clientId);
    static QByteArray getClientKey(const QString &clientId);

private:
    static void loadPairedClients();
    static void savePairedClients();
    static void storeClient(const QString &clientId,
                            const QByteArray &key);

    static QMap<QString, QByteArray> m_clients;
    static bool m_loaded;
};

#endif