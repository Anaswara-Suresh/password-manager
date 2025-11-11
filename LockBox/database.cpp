#include "database.h"
#include <QSqlDatabase>
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
        qDebug() << "FATAL: Database open failed:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery query(m_db);

    // --- Create passwords table ---
    QString createPasswords = R"(
        CREATE TABLE IF NOT EXISTS passwords (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            site BLOB NOT NULL,
            username BLOB NOT NULL,
            password BLOB NOT NULL
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
                           const QByteArray &password_ciphertext)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO passwords (site, username, password) VALUES (?, ?, ?)");
    query.addBindValue(site_ciphertext);
    query.addBindValue(username_ciphertext);
    query.addBindValue(password_ciphertext);

    if (!query.exec()) {
        qDebug() << "Insert failed:" << query.lastError().text();
        return false;
    }

    qDebug() << "Database successfully inserted one encrypted password row.";
    return true;
}
