#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>
#include <QByteArray>

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

    // --- Create passwords table ---
    QString createPasswords = R"(
    CREATE TABLE IF NOT EXISTS passwords (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        site BLOB NOT NULL,
        username BLOB NOT NULL,
        password BLOB NOT NULL,
        entry_hash BLOB UNIQUE NOT NULL
    )
)";
    if (!query.exec(createPasswords)) {
        qDebug() << "Failed to create passwords table:" << query.lastError().text();
        return false;
    }

    // --- Create users table (without email) ---
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

    qDebug() << "Database initialized successfully (users + passwords tables ready)";
    return true;
}

bool Database::addPassword(const QByteArray &site_ciphertext,
                           const QByteArray &username_ciphertext,
                           const QByteArray &password_ciphertext,
                           const QByteArray &entryHash)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO passwords (site, username, password, entry_hash) VALUES (?, ?, ?, ?)");
    query.addBindValue(site_ciphertext);
    query.addBindValue(username_ciphertext);
    query.addBindValue(password_ciphertext);
    query.addBindValue(entryHash);

    if (!query.exec()) {
        QString err = query.lastError().text();
        qDebug() << "Insert failed:" << err;

        if (err.contains("UNIQUE constraint failed")) {
            // Duplicate found — return false gracefully
            return false;
        }
        return false;
    }

    qDebug() << "Database successfully inserted one encrypted password row.";
    return true;
}


QList<QList<QVariant>> Database::fetchAllPasswords() {
    QList<QList<QVariant>> results;
    QSqlQuery query(m_db);
    if (!query.exec("SELECT id, site, username, password FROM passwords")) {
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

bool Database::updatePassword(int id, const QByteArray &site, const QByteArray &user, const QByteArray &pass)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE passwords SET site=?, username=?, password=? WHERE id=?");
    query.addBindValue(site);
    query.addBindValue(user);
    query.addBindValue(pass);
    query.addBindValue(id);
    return query.exec();
}

bool Database::deletePassword(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM passwords WHERE id=?");
    query.addBindValue(id);
    return query.exec();
}

QSqlDatabase& Database::getDatabase()
{
    return m_db;
}
