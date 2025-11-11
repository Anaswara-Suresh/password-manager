#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

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
    QString createTable =
        "CREATE TABLE IF NOT EXISTS passwords ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "site BLOB, username BLOB, password BLOB)";
    if (!query.exec(createTable)) {
        qDebug() << "Failed to create table:" << query.lastError().text();
        return false;
    }
    return true;
}

bool Database::addPassword(const QByteArray &site, const QByteArray &username, const QByteArray &password) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO passwords (site, username, password) VALUES (?, ?, ?)");
    query.addBindValue(site);
    query.addBindValue(username);
    query.addBindValue(password);
    if (!query.exec()) {
        qDebug() << "Insert failed:" << query.lastError().text();
        return false;
    }
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
