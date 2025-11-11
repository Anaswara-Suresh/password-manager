#include "addpasswordpage.h"
#include "ui_addpasswordpage.h"
#include <QMessageBox>
#include "database.h"
#include "utils.h"
#include "crypto.h"
#include <QDebug>
#include <QByteArray>



AddPasswordPage::AddPasswordPage(QWidget *parent, const QByteArray& derivedKey)
    : QMainWindow(parent)
    , ui(new Ui::addpasswordpage)
    , m_derivedKey(derivedKey)
{
    ui->setupUi(this);
    if (m_derivedKey.isEmpty()) {
        qCritical() << "AddPasswordPage started without a valid encryption key!";

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
    qDebug() << "--- STRENGTH CHECK ---";
    qDebug() << "Password retrieved for analysis:" << password;

    int score = Utils::calculatePasswordStrength(password);

    qDebug() << "Calculated Score:" << score;
    qDebug() << "Strength Label:" << Utils::strengthLabel(score);

    ui->strengthLabel->setText("Strength: " + Utils::strengthLabel(score));
}



void AddPasswordPage::on_addButton_clicked()
{
    if (m_derivedKey.isEmpty()) {
        QMessageBox::critical(this, "Security Error",
                              "Cannot save: Encryption key is missing. Please log in again.");
        return;
    }

    QString site_plaintext = ui->siteEdit->text();
    QString username_plaintext = ui->usernameEdit->text();
    QString password_plaintext = ui->passwordEdit->text();

    if (site_plaintext.isEmpty() || username_plaintext.isEmpty() || password_plaintext.isEmpty()) {
        QMessageBox::warning(this, "Error", "All fields are required!");
        return;
    }

    // === Generate unique hash for duplicate checking ===
    QString combined = site_plaintext + ":" + username_plaintext;
    QByteArray combinedBytes = combined.toUtf8();

    QByteArray entryHash(crypto_generichash_BYTES, 0);
    crypto_generichash(reinterpret_cast<unsigned char*>(entryHash.data()),
                       entryHash.size(),
                       reinterpret_cast<const unsigned char*>(combinedBytes.constData()),
                       combinedBytes.size(),
                       nullptr, 0);

    // === Encrypt data ===
    QByteArray site_ciphertext = Crypto::encrypt(site_plaintext, m_derivedKey);
    QByteArray username_ciphertext = Crypto::encrypt(username_plaintext, m_derivedKey);
    QByteArray password_ciphertext = Crypto::encrypt(password_plaintext, m_derivedKey);

    if (site_ciphertext.isEmpty() || username_ciphertext.isEmpty() || password_ciphertext.isEmpty()) {
        QMessageBox::critical(this, "Encryption Error",
                              "Password was NOT encrypted (Ciphertext is empty). Saving aborted.");
        qDebug() << "ERROR: Encryption failed. Check derived key validity.";
        return;
    }

    qDebug() << "Ciphertext Data (HEX - Site):" << site_ciphertext.toHex();

    // === Insert into database (with duplicate prevention) ===
    if (Database::addPassword(site_ciphertext, username_ciphertext, password_ciphertext, entryHash)) {
        QMessageBox::information(this, "Success", "Credentials saved successfully!");
        this->close();
    } else {
        QMessageBox::warning(this, "Duplicate Entry", "These credentials already exist!");
    }
}
