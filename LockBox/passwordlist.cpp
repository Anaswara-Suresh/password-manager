#include "passwordlist.h"
#include "ui_passwordlist.h"
#include "database.h"
#include "crypto.h"
#include "hibpchecker.h"
#include "vault_exporter.h"
#include "vault_importer.h"
#include "totp.h"
#include "totpdialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QHBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QFileDialog>


PasswordList::PasswordList(const QByteArray &key, const QString &username, QWidget *parent)
    : QWidget(parent),
    ui(new Ui::PasswordList),
    masterKey(key),
    currentUsername(username)
{
    ui->setupUi(this);
    ui->verticalLayout->setContentsMargins(15, 15, 15, 15);
    setMinimumSize(700, 400);

    connect(ui->searchButton, &QPushButton::clicked, this, &PasswordList::onSearchClicked);
    connect(ui->searchEdit, &QLineEdit::textChanged, this, &PasswordList::onSearchTextChanged);
    connect(ui->checkAllButton, &QPushButton::clicked, this, &PasswordList::onCheckAllWithHIBP);
    connect(ui->tableWidget, &QTableWidget::cellClicked, this, &PasswordList::onPasswordCellClicked);

    ui->statusLabel->setText("Ready");
    loadPasswords();
}

PasswordList::~PasswordList()
{
    delete ui;
}

void PasswordList::loadPasswords(const QString &filter)
{
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setHorizontalHeaderLabels(
        {"ID", "Site", "Username", "Password", "Actions"});
    ui->tableWidget->setColumnHidden(0, true);
    ui->tableWidget->setRowCount(0);

    QList<QList<QVariant>> rows =
        filter.isEmpty()
            ? Database::fetchAllPasswords(currentUsername)
            : Database::fetchPasswordsBySite(currentUsername, filter);

    int row = 0;
    for (const QList<QVariant> &record : rows) {

        int id = record[0].toInt();
        QString site = record[1].toString();
        QString user = Crypto::decrypt(record[2].toByteArray(), masterKey);
        QString pass = Crypto::decrypt(record[3].toByteArray(), masterKey);

        QByteArray totpCipher = record[4].toByteArray();
        bool totpEnabled = record[5].toInt();

        QString totpSecret;
        if (totpEnabled && !totpCipher.isEmpty())
            totpSecret = Crypto::decrypt(totpCipher, masterKey);

        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(id)));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(site));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(user));

        QTableWidgetItem *pwItem = new QTableWidgetItem("********");
        pwItem->setData(Qt::UserRole, pass);
        ui->tableWidget->setItem(row, 3, pwItem);


        QWidget *actionWidget = new QWidget(this);
        QHBoxLayout *layout = new QHBoxLayout(actionWidget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setAlignment(Qt::AlignCenter);

        QToolButton *settingsBtn = new QToolButton(this);
        settingsBtn->setText("⚙️");
        settingsBtn->setPopupMode(QToolButton::InstantPopup);

        QMenu *menu = new QMenu(settingsBtn);

        // Copy password
        QAction *copyPass = menu->addAction("📋 Copy Password");
        connect(copyPass, &QAction::triggered, this, [=]() {
            emit copyRequested(pass);
            updateStatus("Password copied securely");
        });

        menu->addSeparator();


        if (!totpEnabled) {
            QAction *enableTotp = menu->addAction("🔐 Enable TOTP");
            connect(enableTotp, &QAction::triggered, this, [=]() {
                bool ok;
                QString secret = QInputDialog::getText(
                    this,
                    "Enable TOTP",
                    "Enter TOTP secret (Base32):",
                    QLineEdit::Normal,
                    "",
                    &ok);

                if (!ok || secret.trimmed().isEmpty())
                    return;

                QByteArray cipher = Crypto::encrypt(secret.trimmed(), masterKey);
                if (Database::updateTOTP(currentUsername, id, cipher, 1)) {
                    QMessageBox::information(this, "Success", "TOTP enabled.");
                    loadPasswords();
                }
            });
        } else {
            // Copy TOTP (only if enabled)
            QAction *showTotp = menu->addAction("📟 Show TOTP");
            connect(showTotp, &QAction::triggered, this, [=]() {
                TotpDialog *dlg = new TotpDialog(totpSecret, this);
                dlg->exec();
            });
            QAction *disableTotp = menu->addAction("🚫 Disable TOTP");
            connect(disableTotp, &QAction::triggered, this, [=]() {
                if (QMessageBox::question(
                        this,
                        "Disable TOTP",
                        "Disable TOTP for this entry?")
                    != QMessageBox::Yes)
                    return;

                if (Database::updateTOTP(currentUsername, id, QByteArray(), 0)) {
                    QMessageBox::information(this, "Disabled", "TOTP disabled.");
                    loadPasswords();
                }
            });
        }

        menu->addSeparator();

        QAction *edit = menu->addAction("✏️ Edit");
        edit->setProperty("entryId", id);
        edit->setProperty("entrySite", site);
        connect(edit, &QAction::triggered, this, &PasswordList::onEditButtonClicked);

        QAction *del = menu->addAction("🗑️ Delete");
        del->setProperty("entryId", id);
        connect(del, &QAction::triggered, this, &PasswordList::onDeleteButtonClicked);

        QAction *check = menu->addAction("🔍 Check Breach");
        connect(check, &QAction::triggered, this, [=]() {
            HIBPChecker *checker = new HIBPChecker(this);
            updateStatus("Checking password...");

            connect(checker, &HIBPChecker::resultReady, this, [=](bool pwned, int count) {
                updateStatus(pwned
                                 ? QString("⚠️ Found in %1 breaches").arg(count)
                                 : "✅ Password is safe");
                checker->deleteLater();
            });

            checker->checkPassword(pass);
        });

        settingsBtn->setMenu(menu);
        layout->addWidget(settingsBtn);
        ui->tableWidget->setCellWidget(row, 4, actionWidget);

        row++;
    }

    ui->tableWidget->resizeColumnsToContents();
}

void PasswordList::onEditButtonClicked()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) return;

    int id = action->property("entryId").toInt();
    QString site = action->property("entrySite").toString();

    QString tableName = QString("passwords_%1").arg(currentUsername);
    tableName.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");

    QSqlQuery query(QSqlDatabase::database("lockbox_connection"));
    query.prepare(QString(
                      "SELECT site, username, password FROM %1 WHERE id = :id"
                      ).arg(tableName));
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "Error", "Failed to fetch entry for editing.");
        return;
    }

    QString sitePlain = query.value("site").toString();
    QString userPlain = Crypto::decrypt(
        query.value("username").toByteArray(), masterKey);
    QString passPlain = Crypto::decrypt(
        query.value("password").toByteArray(), masterKey);

    bool ok;
    QString newSite = QInputDialog::getText(
        this, "Edit Site", "Site:",
        QLineEdit::Normal, sitePlain, &ok);
    if (!ok) return;

    QString newUser = QInputDialog::getText(
        this, "Edit Username", "Username:",
        QLineEdit::Normal, userPlain, &ok);
    if (!ok) return;

    QString newPass = QInputDialog::getText(
        this, "Edit Password", "Password:",
        QLineEdit::Normal, passPlain, &ok);
    if (!ok) return;

    // === PASSWORD HISTORY CHECK ===
    QList<QByteArray> history = Database::getPasswordHistory(currentUsername, sitePlain, 5);

    bool isReused = false;
    for (const QByteArray &historicalEncrypted : history) {
        QString historicalPlain = Crypto::decrypt(historicalEncrypted, masterKey);
        if (historicalPlain == newPass) {
            isReused = true;
            break;
        }
    }

    if (isReused) {
        QMessageBox::StandardButton reply = QMessageBox::warning(
            this,
            "Password Reuse Warning",
            "⚠️ This password was previously used for this site.\n\n"
            "Reusing passwords is a security risk. Are you sure you want to continue?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
            );

        if (reply != QMessageBox::Yes) {
            updateStatus("Password update cancelled - reused password");
            return;
        }
    }

    QByteArray siteBytes = newSite.toUtf8();
    QByteArray userCipher = Crypto::encrypt(newUser, masterKey);
    QByteArray passCipher = Crypto::encrypt(newPass, masterKey);

    if (Database::updatePassword(
            currentUsername, id, siteBytes, userCipher, passCipher)) {
        QMessageBox::information(this, "Updated", "Entry updated successfully!");
        loadPasswords();
        updateStatus("Password updated");
    } else {
        QMessageBox::critical(this, "Error", "Failed to update entry.");
    }
}


void PasswordList::onDeleteButtonClicked()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) return;
    int id = action->property("entryId").toInt();

    if (QMessageBox::question(this, "Confirm", "Delete this entry?")
        == QMessageBox::Yes) {
        Database::deletePassword(currentUsername, id);
        loadPasswords();
        updateStatus("Entry deleted");
    }
}

void PasswordList::onSearchClicked()
{
    loadPasswords(ui->searchEdit->text().trimmed());
}

void PasswordList::onSearchTextChanged(const QString &text)
{
    loadPasswords(text.trimmed());
}

void PasswordList::onCheckAllWithHIBP()
{
    updateStatus("Checking all passwords against Have I Been Pwned...");
    int rows = ui->tableWidget->rowCount();

    for (int row = 0; row < rows; ++row) {
        QTableWidgetItem *pwItem = ui->tableWidget->item(row, 3);
        if (!pwItem) continue;

        QString password = pwItem->data(Qt::UserRole).toString();

        HIBPChecker *checker = new HIBPChecker(this);
        checker->setProperty("row", row);

        connect(checker, &HIBPChecker::resultReady, this, [=](bool pwned, int count) {
            int r = checker->property("row").toInt();

            if (pwned) {
                ui->tableWidget->item(r, 3)->setBackground(QColor("#7f1d1d"));
                ui->tableWidget->item(r, 3)->setToolTip(
                    QString("⚠️ Found in data breaches (%1 times)!").arg(count));
            } else {
                ui->tableWidget->item(r, 3)->setBackground(QColor("#1f1f1f"));
                ui->tableWidget->item(r, 3)->setToolTip("✅ Safe");
            }

            if (r == rows - 1)
                ui->statusLabel->setText("All passwords checked");

            checker->deleteLater();
        });

        checker->checkPassword(password);
    }
}

void PasswordList::updateStatus(const QString &msg)
{
    ui->statusLabel->setText(msg);

    QGraphicsOpacityEffect *fx = new QGraphicsOpacityEffect(this);
    ui->statusLabel->setGraphicsEffect(fx);

    QPropertyAnimation *anim = new QPropertyAnimation(fx, "opacity");
    anim->setDuration(300);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QPropertyAnimation::DeleteWhenStopped);
}

void PasswordList::onPasswordCellClicked(int row, int column)
{
    if (column == 3) {
        QTableWidgetItem *item = ui->tableWidget->item(row, column);
        if (!item) return;
        item->setText(item->text() == "********"
                          ? item->data(Qt::UserRole).toString()
                          : "********");
    }
}

void PasswordList::on_btnExportVault_clicked()
{
    QString path = QFileDialog::getSaveFileName(
        this,
        "Export Vault",
        "",
        "LockBox Vault (*.lockbox)"
        );

    if (path.isEmpty())
        return;

    if (!VaultExporter::exportVault(currentUsername, masterKey, path)) {
        QMessageBox::critical(this, "Export Failed",
                              "Could not export vault.");
        return;
    }
    ui->statusLabel->setText("Vault exported successfully");

}

void PasswordList::on_btnImportVault_clicked()
{
    QString path = QFileDialog::getOpenFileName(
        this,
        "Import Vault",
        "",
        "LockBox Vault (*.lockbox)"
        );

    if (path.isEmpty())
        return;

    QString oldUser = QInputDialog::getText(
        this, "Old Username", "Username:");

    if (oldUser.isEmpty())
        return;

    QString oldPass = QInputDialog::getText(
        this,
        "Old Master Password",
        "Password:",
        QLineEdit::Password
        );

    if (oldPass.isEmpty())
        return;

    if (!VaultImporter::importVault(
            path,
            oldUser,
            oldPass,
            masterKey,
            currentUsername)) {

        QMessageBox::critical(this, "Import Failed",
                              "Invalid credentials or corrupted vault.");
        return;
    }

    loadPasswords();
    ui->statusLabel->setText("Vault imported successfully");
}


void PasswordList::refreshPasswords()
{
    loadPasswords();
}

void PasswordList::refreshTable()
{
    loadPasswords();
}
