#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "passwordlist.h"
#include <QMessageBox>
#include "addpasswordpage.h"



MainWindow::MainWindow(const QByteArray &key, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      masterKey(key)
{
    ui->setupUi(this);
    setWindowTitle("🔒 LockBox - Vault");

    connect(ui->viewPasswordsButton, &QPushButton::clicked,
            this, &MainWindow::onViewPasswordsClicked);

    QMessageBox::information(this, "Key Derived", "Master key ready. Vault loaded!");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onViewPasswordsClicked()
{
    PasswordList *listWindow = new PasswordList(this);
    listWindow->setAttribute(Qt::WA_DeleteOnClose);
    listWindow->setWindowTitle("🔐 Saved Passwords");
    listWindow->resize(600, 400);
    listWindow->show();
}

void MainWindow::on_addPasswordButton_clicked()
{
    AddPasswordPage *addPage = new AddPasswordPage(this);
    addPage->setAttribute(Qt::WA_DeleteOnClose);
    addPage->setWindowTitle("➕ Add New Password");
    addPage->resize(500, 400);
    addPage->show();
}

