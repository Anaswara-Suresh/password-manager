#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QByteArray>
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
    void loginSuccessful(const QString &username);

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onResetPasswordClicked();
    void onShowPasswordToggled(bool checked);

private:
    Ui::LoginWindow *ui;

    QByteArray authenticateUser(const QString &username, const QString &password);

    bool registerUser(const QString &username, const QString &password, const QString &email);

    bool resetPassword(const QString &username, const QString &email, const QString &newPassword);

    bool validateUsername(const QString &username);
    bool validatePassword(const QString &password);
    bool validateEmail(const QString &email);
};

#endif
