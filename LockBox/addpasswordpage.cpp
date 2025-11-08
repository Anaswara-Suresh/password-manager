#include "addpasswordpage.h"
#include "ui_addpasswordpage.h"
#include <QMessageBox>
#include "database.h"
#include "utils.h"
#include "crypto.h"
#include <QDebug>
#include <QByteArray>


AddPasswordPage::AddPasswordPage(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::addpasswordpage)
{
    ui->setupUi(this);

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

    QString site_plaintext = ui->siteEdit->text();
    QString username_plaintext = ui->usernameEdit->text();
    QString password_plaintext = ui->passwordEdit->text();

    if (site_plaintext.isEmpty() || username_plaintext.isEmpty() || password_plaintext.isEmpty()) {
        QMessageBox::warning(this, "Error", "All fields are required!");
        return;
    }


    QByteArray site_ciphertext = Crypto::encrypt(site_plaintext);
    QByteArray username_ciphertext = Crypto::encrypt(username_plaintext);
    QByteArray password_ciphertext = Crypto::encrypt(password_plaintext);


    if (site_ciphertext.isEmpty() || username_ciphertext.isEmpty() || password_ciphertext.isEmpty()) {
        QMessageBox::critical(this, "Encryption Error", "Password was NOT encrypted (Ciphertext is empty). Saving aborted.");
        qDebug() << "ERROR: One or more encryption fields failed. Check libsodium linking.";
        return;
    }
    qDebug() << "Ciphertext Data (HEX - Site):" << site_ciphertext.toHex();



    if (Database::addPassword(site_ciphertext, username_ciphertext, password_ciphertext)) {
        QMessageBox::information(this, "Success", "All fields saved successfully (as encrypted BLOBs)!");


    } else {
        QMessageBox::critical(this, "Database Error", "Failed to save password.");
    }
}
