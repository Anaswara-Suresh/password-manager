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


    QString createTable = "CREATE TABLE IF NOT EXISTS passwords ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "site BLOB, "
                          "username BLOB, "
                          "password BLOB)";

    if (!query.exec(createTable)) {
        qDebug() << "Failed to create table:" << query.lastError().text();
        return false;
    }
    return true;
}


bool Database::addPassword(const QByteArray &site_ciphertext, const QByteArray &username_ciphertext, const QByteArray &password_ciphertext) {

    QSqlQuery query(m_db);

    query.prepare("INSERT INTO passwords (site, username, password) VALUES (?, ?, ?)");


    query.addBindValue(site_ciphertext);
    query.addBindValue(username_ciphertext);
    query.addBindValue(password_ciphertext);

    if (!query.exec()) {
        qDebug() << "Insert failed:" << query.lastError().text();
        return false;
    }
    qDebug() << "Database successfully inserted one row (all encrypted).";
    return true;
}
