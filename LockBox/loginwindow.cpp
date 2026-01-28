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
#include "vaultsession.h"
#include "vaultstate.h"
#include "ipcserver.h"

static IPCServer *ipc = nullptr;


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
    if (!ipc) {
        ipc = new IPCServer(this);
        ipc->start();
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
    // Length check
    if (password.length() < 8) {
        QMessageBox::warning(this, "Weak Master Password",
                             "Master password must be at least 8 characters long.");
        return false;
    }

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSymbol = false;

    for (const QChar &ch : password) {
        if (ch.isUpper()) hasUpper = true;
        else if (ch.isLower()) hasLower = true;
        else if (ch.isDigit()) hasDigit = true;
        else hasSymbol = true;   // Anything not alphanumeric is a symbol
    }

    if (!hasUpper || !hasLower || !hasDigit || !hasSymbol) {
        QString message = "Master password must contain:\n";
        if (!hasUpper)  message += "- At least one uppercase letter\n";
        if (!hasLower)  message += "- At least one lowercase letter\n";
        if (!hasDigit)  message += "- At least one number\n";
        if (!hasSymbol) message += "- At least one special symbol\n";

        QMessageBox::warning(this, "Weak Master Password", message);
        return false;
    }

    return true;
}


// ------------------------- AUTHENTICATION -------------------------

QByteArray LoginWindow::authenticateUser(const QString &username,
                                         const QString &password)
{
    QSqlQuery query(QSqlDatabase::database("lockbox_connection"));


    query.prepare(
        "SELECT salt, verification_ciphertext, encrypted_dek "
        "FROM users WHERE username = :username"
        );
    query.bindValue(":username", username);


    if (!query.exec()) {
        qDebug() << "❌ SQL ERROR";
        qDebug() << "Query:" << query.lastQuery();
        qDebug() << "Bound values:" << query.boundValues();
        qDebug() << "Error:" << query.lastError().text();
    }


    if (!query.next()) {
        return QByteArray();
    }

    QByteArray storedSalt        = query.value(0).toByteArray();
    QByteArray storedCiphertext = query.value(1).toByteArray();
    QByteArray encryptedDEK     = query.value(2).toByteArray();

    if (!Crypto::verifyMasterPassword(password, storedSalt, storedCiphertext)) {
        return QByteArray();
    }


    QByteArray kek = Crypto::deriveKey(password, storedSalt);
    if (kek.isEmpty()) {
        return QByteArray();
    }


    QByteArray dek = Crypto::decryptDEK(encryptedDEK, kek);


    kek.fill(0);

    if (dek.isEmpty()) {
        qDebug() << "Failed to decrypt DEK";
        return QByteArray();
    }


    QSqlQuery update(QSqlDatabase::database("lockbox_connection"));
    update.prepare(
        "UPDATE users SET last_login = CURRENT_TIMESTAMP "
        "WHERE username = :username"
        );
    update.bindValue(":username", username);
    update.exec();

    return dek;
}

// ------------------------- REGISTRATION -------------------------

bool LoginWindow::registerUser(const QString &username, const QString &password)
{
    if (!validateUsername(username) || !validatePassword(password))
        return false;


    QByteArray salt(crypto_pwhash_SALTBYTES, 0);
    randombytes_buf(salt.data(), salt.size());


    QByteArray kek = Crypto::deriveKey(password, salt);
    if (kek.isEmpty()) {
        QMessageBox::critical(this, "Error", "Failed to derive encryption key.");
        return false;
    }

    QByteArray dek = Crypto::generateDEK();


    QByteArray encryptedDEK = Crypto::encryptDEK(dek, kek);


    QByteArray verificationCiphertext =
        Crypto::encrypt("MASTER_PASSWORD_VERIFIED", kek);


    dek.fill(0);
    kek.fill(0);

    if (salt.isEmpty() || encryptedDEK.isEmpty() || verificationCiphertext.isEmpty()) {
        QMessageBox::critical(this, "Error",
                              "Failed to perform cryptographic setup. Please try again.");
        return false;
    }


    QSqlQuery query(QSqlDatabase::database("lockbox_connection"));
    query.prepare(
        "INSERT INTO users (username, salt, verification_ciphertext, encrypted_dek, created_at) "
        "VALUES (:username, :salt, :ciphertext, :encrypted_dek, CURRENT_TIMESTAMP)"
        );

    query.bindValue(":username", username);
    query.bindValue(":salt", salt);
    query.bindValue(":ciphertext", verificationCiphertext);
    query.bindValue(":encrypted_dek", encryptedDEK);

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

    QSqlQuery q(QSqlDatabase::database("lockbox_connection"));
    q.prepare(
        "SELECT salt, verification_ciphertext, encrypted_dek "
        "FROM users WHERE username = :u"
        );
    q.bindValue(":u", username);

    if (!q.exec() || !q.next()) {
        QMessageBox::warning(this, "Error", "User not found.");
        return false;
    }

    QByteArray oldSalt        = q.value(0).toByteArray();
    QByteArray oldCipher      = q.value(1).toByteArray();
    QByteArray encryptedDEK   = q.value(2).toByteArray();


    QString oldPassword = QInputDialog::getText(
        this,
        "Verify Old Password",
        "Enter CURRENT master password:",
        QLineEdit::Password
        );

    if (oldPassword.isEmpty())
        return false;


    if (!Crypto::verifyMasterPassword(oldPassword, oldSalt, oldCipher)) {
        QMessageBox::warning(this, "Incorrect Password", "Current password is wrong.");
        return false;
    }


    QByteArray oldKEK = Crypto::deriveKey(oldPassword, oldSalt);
    if (oldKEK.isEmpty()) {
        QMessageBox::critical(this, "Error", "Failed to derive old encryption key.");
        return false;
    }

    QByteArray dek = Crypto::decryptDEK(encryptedDEK, oldKEK);
    oldKEK.fill(0); // 🔥 wipe ASAP

    if (dek.isEmpty()) {
        QMessageBox::critical(this, "Error",
                              "Failed to unlock vault key.\n"
                              "Password reset aborted to prevent data loss.");
        return false;
    }


    QByteArray newSalt(crypto_pwhash_SALTBYTES, 0);
    randombytes_buf(newSalt.data(), newSalt.size());

    QByteArray newKEK = Crypto::deriveKey(newPassword, newSalt);
    if (newKEK.isEmpty()) {
        dek.fill(0);
        QMessageBox::critical(this, "Error", "Failed to derive new encryption key.");
        return false;
    }


    QByteArray newEncryptedDEK = Crypto::encryptDEK(dek, newKEK);


    QByteArray newVerificationCipher =
        Crypto::encrypt("MASTER_PASSWORD_VERIFIED", newKEK);


    dek.fill(0);
    newKEK.fill(0);

    QSqlQuery upd(QSqlDatabase::database("lockbox_connection"));
    upd.prepare(
        "UPDATE users "
        "SET salt = :s, "
        "    verification_ciphertext = :v, "
        "    encrypted_dek = :e "
        "WHERE username = :u"
        );

    upd.bindValue(":s", newSalt);
    upd.bindValue(":v", newVerificationCipher);
    upd.bindValue(":e", newEncryptedDEK);
    upd.bindValue(":u", username);

    if (!upd.exec()) {
        QMessageBox::critical(this, "Database Error",
                              "Failed to update master password.");
        return false;
    }

    QMessageBox::information(
        this,
        "Success",
        "Your master password has been updated successfully.\n"
        "No vault data was re-encrypted."
        );

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
         VaultState::setUnlocked(true);  // vault unlocked
        VaultSession::setSession(username, derivedKey); 
        // Pass username as well
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
    VaultSession::clear();
    VaultState::setUnlocked(false);  // 🔒 Mark vault as locked (auto-lock)

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

