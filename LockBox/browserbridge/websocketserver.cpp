#include "websocketserver.h"
#include "autofillmanager.h"
#include "vaultstate.h"
#include "pairingmanager.h"
#include "browserbridge/aesgcm.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

WebSocketServer::WebSocketServer(QObject *parent)
    : QObject(parent),
      m_server(new QWebSocketServer(
          "LockBox Browser Bridge",
          QWebSocketServer::NonSecureMode,
          this))
{
}

void WebSocketServer::start()
{
    if (m_server->isListening())
        return;

    if (!m_server->listen(QHostAddress::LocalHost, 19455)) {
        qWarning() << "[WS] Failed to start:" << m_server->errorString();
        return;
    }

    connect(m_server, &QWebSocketServer::newConnection,
            this, &WebSocketServer::onNewConnection);

    qDebug() << "[WS] Server started on ws://127.0.0.1:19455";
}

void WebSocketServer::stop()
{
    for (auto client : m_clients) {
        client->close();
    }

    m_clients.clear();
    m_server->close();

    qDebug() << "[WS] Server stopped";
}

bool WebSocketServer::isRunning() const
{
    return m_server->isListening();
}

void WebSocketServer::onNewConnection()
{
    QWebSocket *socket = m_server->nextPendingConnection();
    m_clients.insert(socket);

    connect(socket, &QWebSocket::textMessageReceived,
            this, &WebSocketServer::processTextMessage);

    connect(socket, &QWebSocket::disconnected,
            this, &WebSocketServer::socketDisconnected);

    qDebug() << "[WS] Client connected";
}

void WebSocketServer::processTextMessage(QString message)
{
    qDebug() << "[WS] Received message:" << message;

    QWebSocket *socket = qobject_cast<QWebSocket*>(sender());
    if (!socket)
        return;

    // 🔒 Vault locked check
    if (!VaultState::isUnlocked()) {
        QJsonObject res;
        res["success"] = false;
        res["error"] = "VaultLocked";
        socket->sendTextMessage(QJsonDocument(res).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject())
        return;

    QJsonObject outer = doc.object();
    QJsonObject req;

    QString clientId;
    QByteArray clientKey;

    // =========================================================
    // 🔐 AES ENCRYPTED MESSAGE HANDLING
    // =========================================================
    if (outer.contains("client_id") &&
        outer.contains("nonce") &&
        outer.contains("ciphertext"))
    {
        clientId = outer["client_id"].toString();
        clientKey = PairingManager::getClientKey(clientId);

        if (clientKey.isEmpty()) {
            qDebug() << "[SECURITY] Unknown client";
            return;
        }

        QByteArray nonce =
            QByteArray::fromBase64(outer["nonce"].toString().toUtf8());

        // 🔐 Replay protection
        if (m_lastNonce.contains(clientId) &&
            m_lastNonce[clientId] == nonce)
        {
            qDebug() << "[SECURITY] Replay attack detected";
            return;
        }
        m_lastNonce[clientId] = nonce;

        QByteArray cipher =
            QByteArray::fromBase64(outer["ciphertext"].toString().toUtf8());

        QByteArray decrypted = decryptAESGCM(cipher, nonce, clientKey);

        if (decrypted.isEmpty()) {
            qDebug() << "[SECURITY] AES decrypt failed";
            return;
        }

        QJsonDocument decryptedDoc =
            QJsonDocument::fromJson(decrypted);

        if (!decryptedDoc.isObject()) {
            qDebug() << "[SECURITY] Invalid decrypted payload";
            return;
        }

        req = decryptedDoc.object();

        // 🔐 Inject client_id back after decrypt
        req["client_id"] = clientId;

        qDebug() << "[SECURITY] AES message decrypted";
    }
    else
    {
        // plaintext only allowed for pairing
        req = outer;
    }

    // =========================================================
    // 🔐 TRUST VALIDATION
    // =========================================================
    QString action = req["action"].toString();

    if (action != "pair") {

        if (!req.contains("client_id")) {
            qDebug() << "[SECURITY] Missing client_id";
            return;
        }

        clientId = req["client_id"].toString();

        if (!PairingManager::isClientTrusted(clientId)) {
            qDebug() << "[SECURITY] Untrusted client:" << clientId;
            return;
        }

        clientKey = PairingManager::getClientKey(clientId);
    }

    // =========================================================
    // PAIR
    // =========================================================
    if (action == "pair") {
        QJsonObject res = PairingManager::handlePairRequest();
        socket->sendTextMessage(QJsonDocument(res).toJson(QJsonDocument::Compact));
        return;
    }

    // =========================================================
    // PROCESS ACTION
    // =========================================================
    QJsonObject res;
    QJsonObject payload;

    if (action == "getStatus") {

        payload["vaultLocked"] = false;
        payload["appName"] = "LockBox";
        payload["appVersion"] = "0.2";

        res["success"] = true;
        res["payload"] = payload;
    }

    else if (action == "getCredentials") {

        QString url = req["url"].toString();
        QJsonArray entries =
            AutofillManager::getCredentialsForUrl(url);

        payload["entries"] = entries;

        res["success"] = true;
        res["payload"] = payload;
    }
    else {
        qDebug() << "[WS] Unknown action:" << action;
        return;
    }

    // =========================================================
    // 🔐 ENCRYPT RESPONSE
    // =========================================================
    QByteArray iv;
    QByteArray cipher =
        encryptAESGCM(
            QJsonDocument(res).toJson(QJsonDocument::Compact),
            iv,
            clientKey
        );

    if (cipher.isEmpty()) {
        qDebug() << "[SECURITY] AES response encrypt failed";
        return;
    }

    QJsonObject secure;
    secure["nonce"] = QString(iv.toBase64());
    secure["ciphertext"] = QString(cipher.toBase64());

    socket->sendTextMessage(
        QJsonDocument(secure).toJson(QJsonDocument::Compact)
    );
}

void WebSocketServer::socketDisconnected()
{
    QWebSocket *socket = qobject_cast<QWebSocket*>(sender());
    m_clients.remove(socket);
    socket->deleteLater();

    qDebug() << "[WS] Client disconnected";
}