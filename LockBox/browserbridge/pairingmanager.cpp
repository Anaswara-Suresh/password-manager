#include "pairingmanager.h"

#include <QStandardPaths>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QDebug>
#include <QDir>
#include <QMessageBox>

#include <sodium.h>

// --------------------------------------------------
// Static members
// --------------------------------------------------
QMap<QString, QByteArray> PairingManager::m_clients;
bool PairingManager::m_loaded = false;

// --------------------------------------------------
// Helper: pairing file path
// --------------------------------------------------
static QString getPairingFilePath()
{
    QString dir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QDir().mkpath(dir);
    return dir + "/paired_clients.json";
}

// --------------------------------------------------
// Load paired browsers from disk
// --------------------------------------------------
void PairingManager::loadPairedClients()
{
    if (m_loaded)
        return;

    QFile file(getPairingFilePath());

    if (!file.open(QIODevice::ReadOnly)) {
        m_loaded = true;
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject obj = doc.object();

    for (auto it = obj.begin(); it != obj.end(); ++it) {
        m_clients[it.key()] =
            QByteArray::fromBase64(it.value().toString().toUtf8());
    }

    m_loaded = true;

    qDebug() << "[Pairing] Loaded trusted browsers:" << m_clients.size();
}

// --------------------------------------------------
// Save paired browsers to disk
// --------------------------------------------------
void PairingManager::savePairedClients()
{
    QJsonObject obj;

    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        obj[it.key()] = QString::fromUtf8(it.value().toBase64());
    }

    QFile file(getPairingFilePath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(obj).toJson());
    }
}

// --------------------------------------------------
// Store new trusted client
// --------------------------------------------------
void PairingManager::storeClient(const QString &clientId,
                                 const QByteArray &key)
{
    loadPairedClients();

    m_clients[clientId] = key;

    savePairedClients();

    qDebug() << "[Pairing] Stored new trusted browser:" << clientId;
}

// --------------------------------------------------
// Check if browser is trusted
// --------------------------------------------------
bool PairingManager::isClientTrusted(const QString &clientId)
{
    loadPairedClients();
    return m_clients.contains(clientId);
}

// --------------------------------------------------
// Get AES key for browser
// --------------------------------------------------
QByteArray PairingManager::getClientKey(const QString &clientId)
{
    loadPairedClients();

    if (!m_clients.contains(clientId))
        return QByteArray();

    return m_clients[clientId];
}

// --------------------------------------------------
// Handle new browser pairing
// --------------------------------------------------
QJsonObject PairingManager::handlePairRequest()
{
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(
        nullptr,
        "Browser Pairing Request",
        "A new browser wants to connect to LockBox.\n\nAllow this browser?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        QJsonObject res;
        res["success"] = false;
        res["error"] = "PairingRejected";
        return res;
    }

    // Generate new client ID
    QString clientId =
        QUuid::createUuid().toString(QUuid::WithoutBraces);

    // Generate random 256-bit AES key
    QByteArray key(32, 0);
    randombytes_buf(key.data(), key.size());

    // Store trusted browser
    storeClient(clientId, key);

    qDebug() << "[Pairing] New browser paired:" << clientId;

    // Return pairing response
    QJsonObject res;
    res["success"] = true;
    res["client_id"] = clientId;
    res["key"] = QString(key.toBase64());
    res["type"] = "pair_success";

    return res;
}