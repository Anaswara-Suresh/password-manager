#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QByteArray>

class Database {
private:

    static QSqlDatabase m_db;

public:
    static bool initialize();


    static bool addPassword(const QByteArray &site_ciphertext, const QByteArray &username_ciphertext, const QByteArray &password_ciphertext);
};

#endif
