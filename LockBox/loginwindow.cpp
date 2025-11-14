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
#include <QInputDialog>
#include "mainwindow.h"
#include "autolockmanager.h"

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::LoginWindow),
    autoLockManager(new AutoLockManager(this, 30000))
{
    ui->setupUi(this);
    setWindowTitle("LockBox - Secure Login");
    setFixedSize(450, 550);

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

    connect(autoLockManager, &AutoLockManager::timeout, this, &LoginWindow::handleAutoLock);

    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
    ui->usernameLineEdit->setFocus();

    // Initially stop autolock on login page
    autoLockManager->stop();
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

// ------------------------- VALIDATION -------------------------

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

// ------------------------- AUTHENTICATION -------------------------

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
            QSqlQuery update(QSqlDatabase::database("lockbox_connection"));
            update.prepare("UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE username = :username");
            update.bindValue(":username", username);
            update.exec();

            return derivedKey;
        }
    }

    return QByteArray();
}

// ------------------------- REGISTRATION -------------------------

bool LoginWindow::registerUser(const QString &username, const QString &password)
{
    if (!validateUsername(username) || !validatePassword(password))
        return false;

    QByteArray salt;
    QByteArray verificationCiphertext;

    QByteArray derivedKey = Crypto::registerNewUser(password, salt, verificationCiphertext);

    if (derivedKey.isEmpty() || salt.isEmpty() || verificationCiphertext.isEmpty()) {
        QMessageBox::critical(this, "Error", "Failed to perform cryptographic setup. Please try again.");
        return false;
    }

    QSqlQuery query(QSqlDatabase::database("lockbox_connection"));
    query.prepare("INSERT INTO users (username, salt, verification_ciphertext, created_at) "
                  "VALUES (:username, :salt, :ciphertext, CURRENT_TIMESTAMP)");
    query.bindValue(":username", username);
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


    if (!Database::createUserPasswordTable(username)) {
        QMessageBox::warning(this, "Partial Setup",
                             "User created successfully, but failed to create personal password table.");
    } else {
        qDebug() << "Created password table for user:" << username;
    }

    QMessageBox::information(this, "Registration Successful",
                             "Your account has been created!\nYou can now login.");
    return true;
}

// ------------------------- PASSWORD RESET -------------------------

bool LoginWindow::resetPassword(const QString &username, const QString &newPassword)
{
    if (!validatePassword(newPassword))
        return false;

    QSqlQuery query(QSqlDatabase::database("lockbox_connection"));
    query.prepare("SELECT salt, verification_ciphertext FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "Error", "No matching user found.");
        return false;
    }

    QByteArray storedSalt = query.value(0).toByteArray();
    QByteArray storedCiphertext = query.value(1).toByteArray();

    QString oldPassword = QInputDialog::getText(this, "Verify Old Password",
                                                "Enter your current password:",
                                                QLineEdit::Password);

    if (oldPassword.isEmpty()) return false;

    if (!Crypto::verifyMasterPassword(oldPassword, storedSalt, storedCiphertext)) {
        QMessageBox::warning(this, "Incorrect Password", "Your current password is incorrect.");
        return false;
    }

    QByteArray newSalt, newCipher;
    QByteArray derivedKey = Crypto::registerNewUser(newPassword, newSalt, newCipher);

    if (derivedKey.isEmpty()) {
        QMessageBox::critical(this, "Error", "Failed to generate new password hash.");
        return false;
    }

    QSqlQuery update(QSqlDatabase::database("lockbox_connection"));
    update.prepare("UPDATE users SET salt = :salt, verification_ciphertext = :cipher WHERE username = :username");
    update.bindValue(":salt", newSalt);
    update.bindValue(":cipher", newCipher);
    update.bindValue(":username", username);

    if (!update.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to update password: " + update.lastError().text());
        return false;
    }

    QMessageBox::information(this, "Password Updated",
                             "Your master password has been successfully updated!");
    return true;
}

// ------------------------- BUTTON HANDLERS -------------------------

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

        // ✅ Pass username as well
        MainWindow *mainWindow = new MainWindow(derivedKey, username);
        mainWindow->setAttribute(Qt::WA_DeleteOnClose);
        mainWindow->show();

        // restart auto-lock after login
        autoLockManager->resetTimer();
        autoLockManager->start();
    } else {
        QMessageBox::warning(this, "Login Failed", "Invalid username or password.\nPlease try again.");
    }
}

void LoginWindow::onRegisterClicked()
{
    QString username = ui->usernameLineEdit->text().trimmed();
    QString password = ui->passwordLineEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Registration Failed", "Please fill in all fields.");
        return;
    }

    registerUser(username, password);
}

void LoginWindow::onResetPasswordClicked()
{
    QString username = ui->usernameLineEdit->text().trimmed();
    QString newPassword = ui->passwordLineEdit->text();

    if (username.isEmpty() || newPassword.isEmpty()) {
        QMessageBox::warning(this, "Reset Failed",
                             "Please fill in both username and new password fields.");
        return;
    }

    resetPassword(username, newPassword);
}

void LoginWindow::onShowPasswordToggled(bool checked)
{
    ui->passwordLineEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
}

void LoginWindow::handleAutoLock()
{
    qDebug() << "[AutoLock] Timeout triggered — returning to login screen.";

    // Close all top-level windows except the login page
    for (QWidget *w : QApplication::topLevelWidgets()) {
        if (w != this)
            w->close();
    }

    // Clear sensitive data
    ui->passwordLineEdit->clear();

    // Optionally clear username too
    // ui->usernameLineEdit->clear();

    this->show();
    this->raise();
    this->activateWindow();

    // Stop autolock since we're on login page
    autoLockManager->stop();
}
void LoginWindow::resetFields()
{
    ui->usernameLineEdit->clear();
    ui->passwordLineEdit->clear();
}
