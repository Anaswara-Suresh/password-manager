#ifndef HIBPCHECKER_H
#define HIBPCHECKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCryptographicHash>

class HIBPChecker : public QObject
{
    Q_OBJECT

public:
    explicit HIBPChecker(QObject *parent = nullptr);
    void checkPassword(const QString &password);

signals:
    void resultReady(bool pwned, int count);
    void loginSuccessful(const QByteArray &dataKey);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_networkManager;
    QString m_passwordHashSuffix;
};

#endif // HIBPCHECKER_H
