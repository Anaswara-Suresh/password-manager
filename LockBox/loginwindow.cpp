#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "addpasswordpage.h"
#include "crypto.h"
#include "database.h"
#include "utils.h"
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
        QMessageBox::warning(this, "Weak Master Password", "Master password must be at least 8 characters long.");
        return false;
    }

    int score = Utils::calculatePasswordStrength(password);
    if (score < 5) {
        QMessageBox::warning(this, "Weak Master Password",
                             "Master password must contain uppercase, lowercase, numbers, and symbols.");
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


QByteArray LoginWindow::authenticateUser(const QString &username, const QString &password)
{
    QSqlQuery query(QSqlDatabase::database("lockbox_connection"));

    query.prepare("SELECT salt, verification_ciphertext FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Query error:" << query.lastError().text();
        return QByteArray();
    }

    if (!query.next()) {
        return QByteArray();
    }

    QByteArray storedSalt = query.value(0).toByteArray();
    QByteArray storedCiphertext = query.value(1).toByteArray();


    if (Crypto::verifyMasterPassword(password, storedSalt, storedCiphertext)) {

        QByteArray derivedKey = Crypto::deriveKey(password, storedSalt);

        if (!derivedKey.isEmpty()) {
            // Update last login timestamp
            QSqlQuery update(QSqlDatabase::database("lockbox_connection"));
            update.prepare("UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE username = :username");
            update.bindValue(":username", username);
            update.exec();

            return derivedKey;
        }
    }

    return QByteArray();
}


bool LoginWindow::registerUser(const QString &username, const QString &password, const QString &email)
{
    if (!validateUsername(username) || !validatePassword(password) || !validateEmail(email))
        return false;

    QByteArray salt;
    QByteArray verificationCiphertext;


    QByteArray derivedKey = Crypto::registerNewUser(password, salt, verificationCiphertext);

    if (derivedKey.isEmpty() || salt.isEmpty() || verificationCiphertext.isEmpty()) {
        QMessageBox::critical(this, "Error", "Failed to perform cryptographic setup. Please try again.");
        return false;
    }

    QSqlQuery query(QSqlDatabase::database("lockbox_connection"));

    query.prepare("INSERT INTO users (username, email, salt, verification_ciphertext, created_at) "
                  "VALUES (:username, :email, :salt, :ciphertext, CURRENT_TIMESTAMP)");
    query.bindValue(":username", username);
    query.bindValue(":email", email);
    query.bindValue(":salt", salt);
    query.bindValue(":ciphertext", verificationCiphertext);

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


    QByteArray derivedKey = authenticateUser(username, password);

    if (!derivedKey.isEmpty()) {
        QMessageBox::information(this, "Login Successful", "Welcome back, " + username + "!");
        this->hide();


        AddPasswordPage *page = new AddPasswordPage(nullptr, derivedKey);
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
