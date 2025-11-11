#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QByteArray>
#include "autolockmanager.h"

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

signals:
    void loginSuccessful(const QByteArray &masterKey);

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onResetPasswordClicked();
    void onShowPasswordToggled(bool checked);
    void handleAutoLock();

private:
    Ui::LoginWindow *ui;
    AutoLockManager *autoLockManager;

    QByteArray authenticateUser(const QString &username, const QString &password);

    bool registerUser(const QString &username, const QString &password);
    bool resetPassword(const QString &username, const QString &newPassword);

    bool validateUsername(const QString &username);
    bool validatePassword(const QString &password);
};

#endif
