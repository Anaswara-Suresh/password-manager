#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(const QByteArray &key, QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    masterKey(key)
{
    ui->setupUi(this);
    setWindowTitle("🔒 LockBox - Vault");

    // (optional) you can show a confirmation for now
    QMessageBox::information(this, "Key Derived", "Master key ready. Vault loaded!");
}

MainWindow::~MainWindow()
{
    delete ui;
}
