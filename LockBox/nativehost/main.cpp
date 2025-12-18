// nativehost/main.cpp
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QByteArray>
#include <iostream>
#include <cstdint>
#include <QJsonArray>
#include <QLocalSocket> 
#include "vaultstate.h"


// Read one native message from stdin
QJsonObject readNativeMessage() {
    uint32_t length = 0;

    // Read 4-byte length (little endian)
    std::cin.read(reinterpret_cast<char*>(&length), 4);
    if (!std::cin || length == 0) {
        return QJsonObject(); // empty -> signals end / error
    }

    std::string buffer(length, '\0');
    std::cin.read(&buffer[0], length);
    if (!std::cin) {
        return QJsonObject();
    }

    QByteArray jsonBytes = QByteArray::fromStdString(buffer);
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return QJsonObject();
    }

    return doc.object();
}

// Write one native message to stdout
void writeNativeMessage(const QJsonObject &obj) {
    QJsonDocument doc(obj);
    QByteArray jsonBytes = doc.toJson(QJsonDocument::Compact);
    uint32_t length = static_cast<uint32_t>(jsonBytes.size());

    // Write length
    std::cout.write(reinterpret_cast<const char*>(&length), 4);
    // Write JSON bytes
    std::cout.write(jsonBytes.constData(), length);
    std::cout.flush();
}


QJsonObject callGuiApp(const QJsonObject &req)
{
    QLocalSocket socket;
    socket.connectToServer("lockbox_ipc");

    if (!socket.waitForConnected(200)) {
        QJsonObject err;
        err["success"] = false;
        err["error"] = "GuiNotRunning";
        return err;
    }

    QJsonDocument doc(req);
    QByteArray bytes = doc.toJson(QJsonDocument::Compact);
    socket.write(bytes);
    socket.flush();

    if (!socket.waitForReadyRead(500)) {
        QJsonObject err;
        err["success"] = false;
        err["error"] = "NoResponseFromGui";
        return err;
    }

    QByteArray replyBytes = socket.readAll();
    QJsonParseError parseErr;
    QJsonDocument replyDoc = QJsonDocument::fromJson(replyBytes, &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !replyDoc.isObject()) {
        QJsonObject err;
        err["success"] = false;
        err["error"] = "BadJsonFromGui";
        return err;
    }

    return replyDoc.object();
}



// Here we'll later call your real DB/crypto code.
QJsonObject handleMessage(const QJsonObject &msg) {
    QJsonObject reply;
    reply["version"] = msg.value("version").toInt(1);
    reply["requestId"] = msg.value("requestId").toString("");
    reply["success"] = true;
    reply["error"] = QJsonValue(); // null

    const QString action = msg.value("action").toString();
    QJsonObject payload = msg.value("payload").toObject();
    QJsonObject outPayload;

    if (action == "ping") {
        outPayload["message"] = "pong";
    }
   else if (action == "getStatus") {
    QJsonObject guiReq;
    guiReq["action"] = "getStatus";

    QJsonObject guiRes = callGuiApp(guiReq);

    bool ok = guiRes.value("success").toBool(false);
    if (!ok) {
        reply["success"] = false;
        reply["error"] = guiRes.value("error").toString("GuiNotRunning");
    } else {
        QJsonObject payloadFromGui = guiRes.value("payload").toObject();
        outPayload = payloadFromGui;
    }
}

    else if (action == "getCredentials") {
    QJsonObject guiReq;
    guiReq["action"] = "getCredentials";
    guiReq["url"] = payload.value("url").toString();

    QJsonObject guiRes = callGuiApp(guiReq);

    bool ok = guiRes.value("success").toBool(false);
    if (!ok) {
        reply["success"] = false;
        reply["error"] = guiRes.value("error").toString("GuiNotRunning");
    } else {
        outPayload = guiRes.value("payload").toObject();
    }
}
    else {
        reply["success"] = false;
        reply["error"] = "UnknownAction";
    }

    reply["payload"] = outPayload;
    return reply;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    while (true) {
        QJsonObject msg = readNativeMessage();
        if (msg.isEmpty()) {
            break; // stdin closed or error
        }

        QJsonObject reply = handleMessage(msg);
        writeNativeMessage(reply);
    }

    return 0;
}
