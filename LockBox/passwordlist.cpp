#include "passwordlist.h"
#include "ui_passwordlist.h"
#include "database.h"
#include "crypto.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QHBoxLayout>
#include <QPushButton>

PasswordList::PasswordList(const QByteArray &key, QWidget *parent)
    : QWidget(parent), ui(new Ui::PasswordList), masterKey(key)
{
    ui->setupUi(this);
    loadPasswords();
}

PasswordList::~PasswordList()
{
    delete ui;
}

void PasswordList::loadPasswords()
{
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(5);
    QStringList headers = {"ID", "Site", "Username", "Password", "Actions"};
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    ui->tableWidget->setRowCount(0);

    QList<QList<QVariant>> all = Database::fetchAllPasswords();
    int row = 0;

    for (const QList<QVariant> &record : all) {
        int id = record[0].toInt();
        QByteArray siteCipher = record[1].toByteArray();
        QByteArray userCipher = record[2].toByteArray();
        QByteArray passCipher = record[3].toByteArray();

        QString sitePlain = Crypto::decrypt(siteCipher, masterKey);
        QString userPlain = Crypto::decrypt(userCipher, masterKey);
        QString passPlain = Crypto::decrypt(passCipher, masterKey);

        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(id)));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(sitePlain));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(userPlain));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(passPlain));

        QWidget *actionWidget = new QWidget(this);
        QHBoxLayout *layout = new QHBoxLayout(actionWidget);
        layout->setContentsMargins(0, 0, 0, 0);

        QPushButton *editBtn = new QPushButton("✏️ Edit");
        QPushButton *delBtn = new QPushButton("🗑️ Delete");

        layout->addWidget(editBtn);
        layout->addWidget(delBtn);
        actionWidget->setLayout(layout);

        ui->tableWidget->setCellWidget(row, 4, actionWidget);

        editBtn->setProperty("entryId", id);
        delBtn->setProperty("entryId", id);

        connect(editBtn, &QPushButton::clicked, this, &PasswordList::onEditButtonClicked);
        connect(delBtn, &QPushButton::clicked, this, &PasswordList::onDeleteButtonClicked);

        row++;
    }

    ui->tableWidget->resizeColumnsToContents();
}

void PasswordList::onEditButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    int id = button->property("entryId").toInt();

    QSqlQuery query(QSqlDatabase::database("lockbox_connection"));
    query.prepare("SELECT site, username, password FROM passwords WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "Error", "Failed to fetch entry for editing.");
        return;
    }

    QString sitePlain = Crypto::decrypt(query.value("site").toByteArray(), masterKey);
    QString userPlain = Crypto::decrypt(query.value("username").toByteArray(), masterKey);
    QString passPlain = Crypto::decrypt(query.value("password").toByteArray(), masterKey);

    bool ok;
    QString newSite = QInputDialog::getText(this, "Edit Site (optional)", "Site:", QLineEdit::Normal, sitePlain, &ok);
    if (!ok) return;

    QString newUser = QInputDialog::getText(this, "Edit Username (optional)", "Username:", QLineEdit::Normal, userPlain, &ok);
    if (!ok) return;

    QString newPass = QInputDialog::getText(this, "Edit Password (optional)", "Password:", QLineEdit::Normal, passPlain, &ok);
    if (!ok) return;

    QByteArray siteCipher = newSite.isEmpty() ? query.value("site").toByteArray() : Crypto::encrypt(newSite, masterKey);
    QByteArray userCipher = newUser.isEmpty() ? query.value("username").toByteArray() : Crypto::encrypt(newUser, masterKey);
    QByteArray passCipher = newPass.isEmpty() ? query.value("password").toByteArray() : Crypto::encrypt(newPass, masterKey);

    if (Database::updatePassword(id, siteCipher, userCipher, passCipher)) {
        QMessageBox::information(this, "Updated", "Password entry updated successfully!");
        refreshTable();
    } else {
        QMessageBox::critical(this, "Error", "Failed to update entry.");
    }
}

void PasswordList::onDeleteButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    int id = button->property("entryId").toInt();

    if (QMessageBox::question(this, "Confirm", "Delete this entry?") == QMessageBox::Yes) {
        if (Database::deletePassword(id)) {
            QMessageBox::information(this, "Deleted", "Entry removed successfully.");
            refreshTable();
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete entry.");
        }
    }
}

void PasswordList::refreshTable()
{
    loadPasswords();
}
