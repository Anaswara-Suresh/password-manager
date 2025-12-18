#pragma once

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QJsonObject>

class IPCServer : public QObject
{
    Q_OBJECT
public:
    explicit IPCServer(QObject *parent = nullptr);
    void start();

private:
    QLocalServer *m_server;

    void handleSocket(QLocalSocket *socket);
    QJsonObject handleRequest(const QJsonObject &req);
};
