#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QList>
#include <QVariant>
#include <QByteArray>
#include <QString>

class Database {
private:
    static QSqlDatabase m_db;

public:
    static bool initialize();


    static bool createUserPasswordTable(const QString &username);

    static bool addPassword(const QString &username,
                            const QString &site,
                            const QByteArray &userCipher,
                            const QByteArray &passCipher,
                            const QByteArray &entryHash);

    static QList<QList<QVariant>> fetchAllPasswords(const QString &username);
    static QList<QList<QVariant>> fetchPasswordsBySite(const QString &username, const QString &filter);

    static bool updatePassword(const QString &username, int id,
                               const QByteArray &site, const QByteArray &user, const QByteArray &pass);

    static bool deletePassword(const QString &username, int id);
    static QList<QByteArray> getAllEncryptedPasswords(const QString &owner);
    static QSqlDatabase& getDatabase();
    static QList<QVariantMap> getFullVault(const QString &username);
    static bool updateVaultRow(const QString &username, int id,
                           const QByteArray &cipherUser,
                           const QByteArray &cipherPass);

};

#endif
