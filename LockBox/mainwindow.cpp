#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "passwordlist.h"
#include "addpasswordpage.h"
#include <QMessageBox>

MainWindow::MainWindow(const QByteArray &key, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      masterKey(key)
{
    ui->setupUi(this);
    setWindowTitle("🔒 LockBox - Vault");

    // Connect the buttons in your UI
    connect(ui->viewPasswordsButton, &QPushButton::clicked,
            this, &MainWindow::onViewPasswordsClicked);

    connect(ui->addPasswordButton, &QPushButton::clicked,
            this, &MainWindow::on_addPasswordButton_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onViewPasswordsClicked()
{
    PasswordList *listWindow = new PasswordList(masterKey, this);
    listWindow->setAttribute(Qt::WA_DeleteOnClose);
    listWindow->setWindowTitle("🔐 Saved Passwords");
    listWindow->resize(600, 400);
    listWindow->show();
}

void MainWindow::on_addPasswordButton_clicked()
{
    if (!addPasswordWindow) {
        addPasswordWindow = new AddPasswordPage(this, masterKey);
        addPasswordWindow->setAttribute(Qt::WA_DeleteOnClose);
        addPasswordWindow->setWindowTitle("➕ Add New Password");
        addPasswordWindow->resize(500, 400);

        connect(addPasswordWindow, &QMainWindow::destroyed, this, [this]() {
            addPasswordWindow = nullptr;
        });
    }

    addPasswordWindow->show();
    addPasswordWindow->raise();   
    addPasswordWindow->activateWindow();
}

