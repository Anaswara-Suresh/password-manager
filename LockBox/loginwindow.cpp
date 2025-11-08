#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "crypto.h"
#include "mainwindow.h"
#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);

    // Connect the Login button to our slot
    connect(ui->btnLogin, &QPushButton::clicked, this, &LoginWindow::handleLogin);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::handleLogin()
{
    QString masterPassword = ui->lineEditMasterPass->text();

    if (masterPassword.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter your master password.");
        return;
    }

    // For now, using a fixed salt (later you can generate and store it securely)
    QByteArray salt = QByteArray("LockBoxSalt1234");

    derivedKey = Crypto::deriveKey(masterPassword, salt);

    if (derivedKey.isEmpty()) {
        QMessageBox::critical(this, "Error", "Key derivation failed!");
        return;
    }

    // If successful, open MainWindow and pass key
    MainWindow *mainWin = new MainWindow(derivedKey, this);
    mainWin->show();
    this->close();
}
