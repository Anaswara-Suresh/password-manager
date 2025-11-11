#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "addpasswordpage.h"   // ✅ open after successful login
#include "crypto.h"
#include "database.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QRegularExpression>

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::LoginWindow)
{
    ui->setupUi(this);

    setWindowTitle("LockBox - Secure Login");
    setFixedSize(450, 600);

    if (sodium_init() < 0) {
        QMessageBox::critical(this, "Fatal Error",
                              "Failed to initialize cryptographic library.\nThe application will now close.");
        exit(1);
    }

    if (!Database::initialize()) {
        QMessageBox::critical(this, "Database Error",
                              "Failed to initialize database.");
    }

    connect(ui->loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(ui->registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
    connect(ui->resetPasswordButton, &QPushButton::clicked, this, &LoginWindow::onResetPasswordClicked);
    connect(ui->showPasswordCheckBox, &QCheckBox::toggled, this, &LoginWindow::onShowPasswordToggled);

    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
    ui->usernameLineEdit->setFocus();
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

bool LoginWindow::validateUsername(const QString &username)
{
    if (username.length() < 3 || username.length() > 30) {
        QMessageBox::warning(this, "Invalid Username", "Username must be between 3 and 30 characters.");
        return false;
    }
    QRegularExpression regex("^[a-zA-Z0-9_]+$");
    if (!regex.match(username).hasMatch()) {
        QMessageBox::warning(this, "Invalid Username",
                             "Username can only contain letters, numbers, and underscores.");
        return false;
    }
    return true;
}

bool LoginWindow::validatePassword(const QString &password)
{
    if (password.length() < 8) {
        QMessageBox::warning(this, "Weak Password", "Password must be at least 8 characters long.");
        return false;
    }
    return true;
}

bool LoginWindow::validateEmail(const QString &email)
{
    QRegularExpression regex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    if (!regex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Invalid Email", "Please enter a valid email address.");
        return false;
    }
    return true;
}

bool LoginWindow::authenticateUser(const QString &username, const QString &password)
{
    QSqlQuery query(QSqlDatabase::database("lockbox_connection"));
    query.prepare("SELECT password_hash FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Query error:" << query.lastError().text();
        return false;
    }

    if (!query.next()) {
        return false; // user not found
    }

    QByteArray storedHash = query.value(0).toByteArray();

    if (Crypto::verifyPassword(password, storedHash)) {
        QSqlQuery update(QSqlDatabase::database("lockbox_connection"));
        update.prepare("UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE username = :username");
        update.bindValue(":username", username);
        update.exec();
        return true;
    }

    return false;
}

bool LoginWindow::registerUser(const QString &username, const QString &password, const QString &email)
{
    if (!validateUsername(username) || !validatePassword(password) || !validateEmail(email))
        return false;

    QByteArray hash = Crypto::hashPassword(password);
    if (hash.isEmpty()) {
        QMessageBox::critical(this, "Error", "Failed to hash password. Please try again.");
        return false;
    }

    QSqlQuery query(QSqlDatabase::database("lockbox_connection"));
    query.prepare("INSERT INTO users (username, email, password_hash, created_at) "
                  "VALUES (:username, :email, :hash, CURRENT_TIMESTAMP)");
    query.bindValue(":username", username);
    query.bindValue(":email", email);
    query.bindValue(":hash", hash);

    if (!query.exec()) {
        QString err = query.lastError().text();
        if (err.contains("UNIQUE constraint failed")) {
            QMessageBox::warning(this, "Registration Failed",
                                 "This username is already taken. Please choose another.");
        } else {
            QMessageBox::critical(this, "Error", "Database error: " + err);
        }
        return false;
    }

    QMessageBox::information(this, "Registration Successful",
                             "Your account has been created!\nYou can now login.");
    return true;
}

void LoginWindow::onLoginClicked()
{
    QString username = ui->usernameLineEdit->text().trimmed();
    QString password = ui->passwordLineEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Login Failed", "Please enter both username and password.");
        return;
    }

    if (authenticateUser(username, password)) {
        QMessageBox::information(this, "Login Successful", "Welcome back, " + username + "!");
        this->hide();

        AddPasswordPage *page = new AddPasswordPage();
        page->show();
    } else {
        QMessageBox::warning(this, "Login Failed", "Invalid username or password.\nPlease try again.");
    }
}

void LoginWindow::onRegisterClicked()
{
    QString username = ui->usernameLineEdit->text().trimmed();
    QString password = ui->passwordLineEdit->text();
    QString email = ui->emailLineEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "Registration Failed", "Please fill in all fields.");
        return;
    }

    registerUser(username, password, email);
}

void LoginWindow::onResetPasswordClicked()
{
    QMessageBox::information(this, "Not Implemented Yet",
                             "Password reset will be added soon.");
}

void LoginWindow::onShowPasswordToggled(bool checked)
{
    ui->passwordLineEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
}
