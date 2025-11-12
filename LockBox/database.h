#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QList>
#include <QVariant>
#include <QByteArray>

class Database {
private:
    static QSqlDatabase m_db;

public:
    static bool initialize();
    static bool addPassword(const QString &site, const QByteArray &username, const QByteArray &password, const QByteArray &entryHash);

    static QList<QList<QVariant>> fetchAllPasswords();
    static QList<QList<QVariant>> fetchPasswordsBySite(const QString &filter);


    static bool updatePassword(int id, const QByteArray &site, const QByteArray &user, const QByteArray &pass);
    static bool deletePassword(int id);


    static QSqlDatabase& getDatabase();
};

#endif
