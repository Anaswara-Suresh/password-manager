#pragma once

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QSet>


class WebSocketServer : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketServer(QObject *parent = nullptr);
    void start();
    void stop();
    bool isRunning() const;

private slots:
    void onNewConnection();
    void processTextMessage(QString message);
    void socketDisconnected();

private:
    QWebSocketServer *m_server;
    QSet<QWebSocket*> m_clients;
    QHash<QString, QByteArray> m_lastNonce;
};