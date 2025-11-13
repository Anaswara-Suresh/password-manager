#include "hibpchecker.h"
#include <QDebug>
#include <QStringList>

HIBPChecker::HIBPChecker(QObject *parent)
    : QObject(parent),
    m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &HIBPChecker::onReplyFinished);
}

void HIBPChecker::checkPassword(const QString &password)
{
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha1).toHex().toUpper();
    QString prefix = QString::fromLatin1(hash.left(5));
    m_passwordHashSuffix = QString::fromLatin1(hash.mid(5));

    QUrl url(QStringLiteral("https://api.pwnedpasswords.com/range/") + prefix);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "LockBox/1.0 (Qt HIBP Integration)");
    m_networkManager->get(request);
}

void HIBPChecker::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "HIBP request failed:" << reply->errorString();
        emit resultReady(false, 0);
        reply->deleteLater();
        return;
    }

    QByteArray response = reply->readAll();
    reply->deleteLater();

    // Parse response
    QStringList lines = QString::fromUtf8(response).split('\n');
    for (const QString &line : lines) {
        QStringList parts = line.split(':');
        if (parts.size() == 2) {
            QString suffix = parts[0].trimmed();
            int count = parts[1].trimmed().toInt();
            if (suffix.compare(m_passwordHashSuffix, Qt::CaseInsensitive) == 0) {
                emit resultReady(true, count);
                return;
            }
        }
    }

    emit resultReady(false, 0);
}
