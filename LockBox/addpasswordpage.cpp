#include "addpasswordpage.h"
#include "ui_addpasswordpage.h"
#include <QMessageBox>
#include "database.h"
#include "utils.h"
#include "crypto.h"
#include <QDebug>
#include <QByteArray>

AddPasswordPage::AddPasswordPage(QWidget *parent,
                                 const QByteArray &derivedKey,
                                 const QString &currentUsername)
    : QMainWindow(parent),
    ui(new Ui::addpasswordpage),
    m_derivedKey(derivedKey),
    m_currentUsername(currentUsername)
{
    ui->setupUi(this);

    if (m_derivedKey.isEmpty()) {
        qCritical() << "AddPasswordPage started without a valid encryption key!";
    }

    if (m_currentUsername.isEmpty()) {
        qWarning() << "Warning: AddPasswordPage opened without a username!";
    }
}

AddPasswordPage::~AddPasswordPage()
{
    delete ui;
}

void AddPasswordPage::on_generateButton_clicked()
{
    QString newPassword = Utils::generatePassword(12);
    ui->passwordEdit->setText(newPassword);
}

void AddPasswordPage::on_analyzeButton_clicked()
{
    QString password = ui->passwordEdit->text();
    int score = Utils::calculatePasswordStrength(password);
    ui->strengthLabel->setText("Strength: " + Utils::strengthLabel(score));
}

void AddPasswordPage::on_addButton_clicked()
{
    if (m_derivedKey.isEmpty()) {
        QMessageBox::critical(this, "Security Error",
                              "Cannot save: Encryption key is missing. Please log in again.");
        return;
    }

    if (m_currentUsername.isEmpty()) {
        QMessageBox::critical(this, "User Error",
                              "User information missing. Please log in again.");
        return;
    }

    QString site_plaintext = ui->siteEdit->text();
    QString username_plaintext = ui->usernameEdit->text();
    QString password_plaintext = ui->passwordEdit->text();

    if (site_plaintext.isEmpty() || username_plaintext.isEmpty() || password_plaintext.isEmpty()) {
        QMessageBox::warning(this, "Error", "All fields are required!");
        return;
    }

    // === Unique entry hash ===
    QString combined = site_plaintext + ":" + username_plaintext;
    QByteArray combinedBytes = combined.toUtf8();
    QByteArray entryHash(crypto_generichash_BYTES, 0);
    crypto_generichash(reinterpret_cast<unsigned char*>(entryHash.data()),
                       entryHash.size(),
                       reinterpret_cast<const unsigned char*>(combinedBytes.constData()),
                       combinedBytes.size(),
                       nullptr, 0);

    // === Encrypt ===
    QByteArray username_ciphertext = Crypto::encrypt(username_plaintext, m_derivedKey);
    QByteArray password_ciphertext = Crypto::encrypt(password_plaintext, m_derivedKey);

    if (username_ciphertext.isEmpty() || password_ciphertext.isEmpty()) {
        QMessageBox::critical(this, "Encryption Error",
                              "Password or username encryption failed. Saving aborted.");
        return;
    }

    // === Insert into the logged-in user's table ===
    if (Database::addPassword(m_currentUsername, site_plaintext,
                              username_ciphertext, password_ciphertext, entryHash)) {
        QMessageBox::information(this, "Success", "Credentials saved successfully!");
        this->close();
    } else {
        QMessageBox::warning(this, "Duplicate Entry", "These credentials already exist!");
    }
}
