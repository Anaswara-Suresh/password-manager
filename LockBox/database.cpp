#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>
#include <QRegularExpression>


QSqlDatabase Database::m_db;

bool Database::initialize() {
    if (QSqlDatabase::contains("lockbox_connection")) {
        m_db = QSqlDatabase::database("lockbox_connection");
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE", "lockbox_connection");
        m_db.setDatabaseName("lockbox.db");
    }

    if (!m_db.open()) {
        qDebug() << "Database open failed:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery query(m_db);

    // Users table
    QString createUsers = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            salt BLOB NOT NULL,
            verification_ciphertext BLOB NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            last_login TIMESTAMP
        )
    )";

    if (!query.exec(createUsers)) {
        qDebug() << "Failed to create users table:" << query.lastError().text();
        return false;
    }

    qDebug() << "Database initialized successfully (users table ready)";
    return true;
}

bool Database::createUserPasswordTable(const QString &username) {
    QString tableName = QString("passwords_%1").arg(username);
    tableName.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");


    QString createTable = QString(R"(
        CREATE TABLE IF NOT EXISTS %1 (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            site TEXT NOT NULL,
            username BLOB NOT NULL,
            password BLOB NOT NULL,
            entry_hash BLOB UNIQUE NOT NULL
        )
    )").arg(tableName);

    QSqlQuery query(m_db);
    if (!query.exec(createTable)) {
        qDebug() << "Failed to create user table" << tableName << ":" << query.lastError().text();
        return false;
    }

    qDebug() << "Created table for user:" << tableName;
    return true;
}
QList<QByteArray> Database::getAllEncryptedPasswords(const QString &owner) {
    QList<QByteArray> list;

    QString tableName = QString("passwords_%1").arg(owner);
    tableName.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");

    QSqlQuery query(m_db);
    query.prepare(QString("SELECT password FROM %1").arg(tableName));

    if (!query.exec()) {
        qWarning() << "Failed to load encrypted passwords:" << query.lastError();
        return list;
    }

    while (query.next()) {
        list.append(query.value(0).toByteArray());
    }

    return list;
}


bool Database::addPassword(const QString &username,
                           const QString &site,
                           const QByteArray &userCipher,
                           const QByteArray &passCipher,
                           const QByteArray &entryHash)
{
    QString tableName = QString("passwords_%1").arg(username);
    tableName.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");


    QSqlQuery query(m_db);
    query.prepare(QString("INSERT INTO %1 (site, username, password, entry_hash) VALUES (?, ?, ?, ?)").arg(tableName));
    query.addBindValue(site);
    query.addBindValue(userCipher);
    query.addBindValue(passCipher);
    query.addBindValue(entryHash);

    if (!query.exec()) {
        qDebug() << "Insert failed for" << tableName << ":" << query.lastError().text();
        return false;
    }

    qDebug() << "Password added to" << tableName;
    return true;
}

QList<QList<QVariant>> Database::fetchAllPasswords(const QString &username) {
    QList<QList<QVariant>> results;
    QString tableName = QString("passwords_%1").arg(username);
    tableName.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");


    QSqlQuery query(m_db);
    if (!query.exec(QString("SELECT id, site, username, password FROM %1").arg(tableName))) {
        qDebug() << "Fetch failed:" << query.lastError().text();
        return results;
    }

    while (query.next()) {
        QList<QVariant> row;
        row << query.value("id")
            << query.value("site")
            << query.value("username")
            << query.value("password");
        results << row;
    }
    return results;
}

QList<QList<QVariant>> Database::fetchPasswordsBySite(const QString &username, const QString &filter) {
    QList<QList<QVariant>> results;
    QString tableName = QString("passwords_%1").arg(username);
    tableName.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");

    QSqlQuery query(m_db);
    query.prepare(QString("SELECT id, site, username, password FROM %1 WHERE site LIKE :pattern").arg(tableName));
    query.bindValue(":pattern", "%" + filter + "%");

    if (!query.exec()) {
        qCritical() << "Database search failed:" << query.lastError().text();
        return results;
    }

    while (query.next()) {
        QList<QVariant> row;
        row << query.value("id")
            << query.value("site")
            << query.value("username")
            << query.value("password");
        results.append(row);
    }
    return results;
}

bool Database::updatePassword(const QString &username, int id,
                              const QByteArray &site, const QByteArray &user, const QByteArray &pass)
{
    QString tableName = QString("passwords_%1").arg(username);
    tableName.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");


    QSqlQuery query(m_db);
    query.prepare(QString("UPDATE %1 SET site=?, username=?, password=? WHERE id=?").arg(tableName));
    query.addBindValue(site);
    query.addBindValue(user);
    query.addBindValue(pass);
    query.addBindValue(id);

    return query.exec();
}

bool Database::deletePassword(const QString &username, int id) {
    QString tableName = QString("passwords_%1").arg(username);
    tableName.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");


    QSqlQuery query(m_db);
    query.prepare(QString("DELETE FROM %1 WHERE id=?").arg(tableName));
    query.addBindValue(id);

    return query.exec();
}

QSqlDatabase& Database::getDatabase() {
    return m_db;
}

QList<QVariantMap> Database::getFullVault(const QString &username)
{
    QList<QVariantMap> list;
    QString table = QString("passwords_%1").arg(username).replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");

    QSqlQuery q(m_db);
    q.prepare(QString("SELECT id, site, username, password FROM %1").arg(table));

    if (!q.exec())
        return list;

    while (q.next()) {
        QVariantMap row;
        row["id"] = q.value("id");
        row["site"] = q.value("site");
        row["username"] = q.value("username");
        row["password"] = q.value("password");
        list.append(row);
    }

    return list;
}


bool Database::updateVaultRow(const QString &username, int id,
                              const QByteArray &cipherUser,
                              const QByteArray &cipherPass)
{
    QString table = QString("passwords_%1").arg(username).replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");

    QSqlQuery q(m_db);
    q.prepare(QString("UPDATE %1 SET username = ?, password = ? WHERE id = ?").arg(table));
    q.addBindValue(cipherUser);
    q.addBindValue(cipherPass);
    q.addBindValue(id);

    return q.exec();
}

