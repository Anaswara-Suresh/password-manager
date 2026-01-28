#include "ipcserver.h"
#include <QJsonDocument>
#include <QDebug>
#include "autofillmanager.h"
#include "vaultstate.h"      // to check locked/unlocked

IPCServer::IPCServer(QObject *parent)
    : QObject(parent),
      m_server(new QLocalServer(this))
{
}

void IPCServer::start()
{
    QString socketName = "lockbox_ipc";

    // Remove old socket if it exists
    QLocalServer::removeServer(socketName);

    if (!m_server->listen(socketName)) {
        qWarning() << "[IPC] Failed to listen on" << socketName << ":" << m_server->errorString();
        return;
    }

    QObject::connect(m_server, &QLocalServer::newConnection, this, [this]() {
        while (m_server->hasPendingConnections()) {
            QLocalSocket *socket = m_server->nextPendingConnection();
            handleSocket(socket);
        }
    });

    qDebug() << "[IPC] Server started on" << socketName;
}

void IPCServer::handleSocket(QLocalSocket *socket)
{
    QObject::connect(socket, &QLocalSocket::readyRead, this, [this, socket]() {
        QByteArray data = socket->readAll();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            qWarning() << "[IPC] Invalid JSON from client";
            socket->disconnectFromServer();
            return;
        }

        QJsonObject req = doc.object();
        QJsonObject res = handleRequest(req);

        QJsonDocument outDoc(res);
        QByteArray outBytes = outDoc.toJson(QJsonDocument::Compact);
        socket->write(outBytes);
        socket->flush();
        socket->disconnectFromServer();
    });

    QObject::connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
}

QJsonObject IPCServer::handleRequest(const QJsonObject &req)
{
    QJsonObject res;
    QString action = req.value("action").toString();

    if (action == "getStatus") {
        bool vaultLocked = !VaultState::isUnlocked();

        QJsonObject payload;
        payload["vaultLocked"] = vaultLocked;
        payload["appName"] = "LockBox";
        payload["appVersion"] = "0.1.0";

        res["success"] = true;
        res["error"] = QJsonValue(); // null
        res["payload"] = payload;
    } else {
        res["success"] = false;
        res["error"] = "UnknownAction";
        res["payload"] = QJsonObject();
    }
    if (action == "getCredentials") {
        QString url = req.value("url").toString();
        QJsonArray entries = AutofillManager::getCredentialsForUrl(url);

        QJsonObject payload;
        payload["entries"] = entries;

        res["success"] = true;
        res["error"] = QJsonValue();
        res["payload"] = payload;
        return res;
    }


    return res;
}
