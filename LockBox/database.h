#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QList>
#include <QVariant>
#include <QVariantMap>
#include <QByteArray>
#include <QString>

class Database
{
private:
    static QSqlDatabase m_db;

public:
    /* -------- Core -------- */
    static bool initialize();
    static QSqlDatabase &getDatabase();

    /* -------- User / Tables -------- */
    static bool createUserPasswordTable(const QString &username);

    /* -------- Password CRUD -------- */
    static bool addPassword(const QString &username,
                            const QString &site,
                            const QByteArray &userCipher,
                            const QByteArray &passCipher,
                            const QByteArray &entryHash);

    static QList<QList<QVariant>> fetchAllPasswords(const QString &username);
    static QList<QList<QVariant>> fetchPasswordsBySite(const QString &username,
                                                       const QString &filter);

    static bool updatePassword(const QString &username,
                               int id,
                               const QString &site,
                               const QByteArray &user,
                               const QByteArray &pass);

    static bool updateTOTP(const QString &username,
                           int id,
                           const QByteArray &totpCipher,
                           int enabled);

    static bool deletePassword(const QString &username, int id);
    static bool storeEncryptedDEK(const QString &username,
                                  const QByteArray &encryptedDek);

    static QByteArray fetchEncryptedDEK(const QString &username);

    /* -------- Vault / Export -------- */
    static QList<QByteArray> getAllEncryptedPasswords(const QString &owner);
    static QList<QVariantMap> getFullVault(const QString &username);

    static bool updateVaultRow(const QString &username,
                               int id,
                               const QByteArray &cipherUser,
                               const QByteArray &cipherPass);

    static QList<QVariantMap> findBySite(const QString &username,
                                         const QString &site);

    /* -------- Transactions (Import / Export safety) -------- */
    static bool beginTransaction();
    static void commit();
    static void rollback();

    /* -------- Password History -------- */
    static bool addPasswordHistory(const QString &username,
                                   const QString &site,
                                   const QByteArray &passwordCipher);

    static QList<QByteArray> getPasswordHistory(const QString &username,
                                                const QString &site,
                                                int limit = 5);

    static bool createPasswordHistoryTable(const QString &username);
};

#endif // DATABASE_H
