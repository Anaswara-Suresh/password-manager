#include "passwordlist.h"
#include "ui_passwordlist.h"
#include "database.h"
#include "crypto.h"
#include "hibpchecker.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

PasswordList::PasswordList(const QByteArray &key, const QString &username, QWidget *parent)
    : QWidget(parent), ui(new Ui::PasswordList), masterKey(key), currentUsername(username)
{
    ui->setupUi(this);
    ui->verticalLayout->setContentsMargins(15, 15, 15, 15);
    this->setMinimumSize(700, 400);
    ui->tableWidget->resizeColumnsToContents();

    ui->tableWidget->setColumnWidth(4, qMax(300, ui->tableWidget->columnWidth(4)));

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
    QStringList headers = {"ID", "Site", "Username", "Password", "Actions"};
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    ui->tableWidget->setColumnHidden(0, true);
    ui->tableWidget->setRowCount(0);

    QList<QList<QVariant>> all;

    if (filter.isEmpty()) {
        all = Database::fetchAllPasswords(currentUsername);
    } else {
        all = Database::fetchPasswordsBySite(currentUsername, filter);
    }

    int row = 0;
    for (const QList<QVariant> &record : all) {
        int id = record[0].toInt();
        QByteArray siteBytes = record[1].toByteArray();
        QByteArray userCipher = record[2].toByteArray();
        QByteArray passCipher = record[3].toByteArray();

        QString sitePlain = QString::fromUtf8(siteBytes);
        QString userPlain = Crypto::decrypt(userCipher, masterKey);
        QString passPlain = Crypto::decrypt(passCipher, masterKey);

        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(id)));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(sitePlain));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(userPlain));
        
        QTableWidgetItem *passwordItem = new QTableWidgetItem("********");
        passwordItem->setData(Qt::UserRole, passPlain);
        ui->tableWidget->setItem(row, 3, passwordItem);

        QWidget *actionWidget = new QWidget(this);
        QHBoxLayout *layout = new QHBoxLayout(actionWidget);
        layout->setContentsMargins(0, 0, 0, 0);

        QPushButton *editBtn = new QPushButton("✏️ Edit");
        QPushButton *delBtn = new QPushButton("🗑️ Delete");
        QPushButton *checkBtn = new QPushButton("🔍 Check");
        QPushButton *copyBtn  = new QPushButton("📋 Copy");

        layout->addWidget(editBtn);
        layout->addWidget(delBtn);
        layout->addWidget(checkBtn);
        layout->addWidget(copyBtn);
        layout->setSpacing(8);
        layout->setAlignment(Qt::AlignCenter);
        ui->tableWidget->setCellWidget(row, 4, actionWidget);

        editBtn->setProperty("entryId", id);
        delBtn->setProperty("entryId", id);
        copyBtn->setProperty("row", row);

        connect(copyBtn, &QPushButton::clicked, this, [=]() {
            int r = copyBtn->property("row").toInt();
            QString password = ui->tableWidget
                                   ->item(r, 3)
                                   ->data(Qt::UserRole)
                                   .toString();

            emit copyRequested(password);
            updateStatus("Password copied securely (auto-clears)");
        });

        connect(editBtn, &QPushButton::clicked, this, &PasswordList::onEditButtonClicked);
        connect(delBtn, &QPushButton::clicked, this, &PasswordList::onDeleteButtonClicked);

        checkBtn->setProperty("row", row);

        connect(checkBtn, &QPushButton::clicked, this, [=]() {
            int realRow = checkBtn->property("row").toInt();
            QString password = ui->tableWidget->item(realRow, 3)->data(Qt::UserRole).toString();

            HIBPChecker *checker = new HIBPChecker(this);
            ui->statusLabel->setText(QString("Checking password for %1...").arg(sitePlain));

            connect(checker, &HIBPChecker::resultReady, this, [=](bool pwned, int count) {
                int r = checkBtn->property("row").toInt();

                if (pwned) {
                    ui->tableWidget->item(r, 3)->setBackground(QColor("#7f1d1d"));
                    ui->tableWidget->item(r, 3)->setToolTip(
                        QString("⚠️ Found in data breaches (%1 times)!").arg(count));
                } else {
                    ui->tableWidget->item(r, 3)->setBackground(QColor("#1f1f1f"));
                    ui->tableWidget->item(r, 3)->setToolTip("✅ Safe (no known breaches)");
                }
                ui->statusLabel->setText("Check complete ");
                checker->deleteLater();
            });

            checker->checkPassword(password);
        });

        row++;
    }

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->setColumnWidth(4, qMax(300, ui->tableWidget->columnWidth(4)));
}

void PasswordList::onEditButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    int id = button->property("entryId").toInt();
    QString tableName = QString("passwords_%1").arg(currentUsername);

    QSqlQuery query(QSqlDatabase::database("lockbox_connection"));
    query.prepare(QString("SELECT site, username, password FROM %1 WHERE id = :id").arg(tableName));
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "Error", "Failed to fetch entry for editing.");
        return;
    }

    QString sitePlain = QString::fromUtf8(query.value("site").toByteArray());
    QString userPlain = Crypto::decrypt(query.value("username").toByteArray(), masterKey);
    QString passPlain = Crypto::decrypt(query.value("password").toByteArray(), masterKey);

    bool ok;
    QString newSite = QInputDialog::getText(this, "Edit Site", "Site:", QLineEdit::Normal, sitePlain, &ok);
    if (!ok) return;

    QString newUser = QInputDialog::getText(this, "Edit Username", "Username:", QLineEdit::Normal, userPlain, &ok);
    if (!ok) return;

    QString newPass = QInputDialog::getText(this, "Edit Password", "Password:", QLineEdit::Normal, passPlain, &ok);
    if (!ok) return;

    QByteArray siteBytes = newSite.toUtf8();
    QByteArray userCipher = Crypto::encrypt(newUser, masterKey);
    QByteArray passCipher = Crypto::encrypt(newPass, masterKey);

    if (Database::updatePassword(currentUsername, id, siteBytes, userCipher, passCipher)) {
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
        if (Database::deletePassword(currentUsername, id)) {
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

void PasswordList::refreshPasswords()
{
    loadPasswords();
}

void PasswordList::onSearchClicked()
{
    QString filter = ui->searchEdit->text().trimmed();
    loadPasswords(filter);
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
                ui->statusLabel->setText("All passwords checked ");

            checker->deleteLater();
        });

        checker->checkPassword(password);
    }
}

void PasswordList::updateStatus(const QString &message)
{
    ui->statusLabel->setText(message);

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(this);
    ui->statusLabel->setGraphicsEffect(effect);

    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(300);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->start(QPropertyAnimation::DeleteWhenStopped);
}

void PasswordList::onPasswordCellClicked(int row, int column)
{
    if (column == 3) {
        QTableWidgetItem *item = ui->tableWidget->item(row, column);
        if (item) {
            if (item->text() == "********") {
                item->setText(item->data(Qt::UserRole).toString());
            } else {
                item->setText("********");
            }
        }
    }
}

