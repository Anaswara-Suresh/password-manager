#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "passwordlist.h"
#include "addpasswordpage.h"
#include <QMessageBox>
#include "loginwindow.h"

MainWindow::MainWindow(const QByteArray &key, const QString &username, QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    masterKey(key),
    currentUser(username)
{
    ui->setupUi(this);
    setWindowTitle("🔒 LockBox - Vault (" + username + ")");

    connect(ui->viewPasswordsButton, &QPushButton::clicked,
            this, &MainWindow::onViewPasswordsClicked);
    connect(ui->addPasswordButton, &QPushButton::clicked,
            this, &MainWindow::on_addPasswordButton_clicked);
    connect(ui->btnLogout, &QPushButton::clicked,
            this, &MainWindow::on_btnLogout_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onViewPasswordsClicked()
{
    PasswordList *listWindow = new PasswordList(masterKey, currentUser, this);
    listWindow->setAttribute(Qt::WA_DeleteOnClose);
    listWindow->setWindowTitle("🔐 " + currentUser + "'s Saved Passwords");
    listWindow->resize(600, 400);
    listWindow->show();
}

void MainWindow::on_addPasswordButton_clicked()
{
    if (!addPasswordWindow) {
        addPasswordWindow = new AddPasswordPage(this, masterKey, currentUser);
        addPasswordWindow->setAttribute(Qt::WA_DeleteOnClose);
        addPasswordWindow->setWindowTitle("➕ Add New Password (" + currentUser + ")");
        addPasswordWindow->resize(500, 400);

        connect(addPasswordWindow, &QMainWindow::destroyed, this, [this]() {
            addPasswordWindow = nullptr;
        });
    }

    addPasswordWindow->show();
    addPasswordWindow->raise();
    addPasswordWindow->activateWindow();
}

void MainWindow::on_btnLogout_clicked()
{
    QWidgetList widgets = QApplication::topLevelWidgets();
    for (QWidget *w : widgets) {
        if (w->inherits("LoginWindow")) {

            // Cast to LoginWindow
            LoginWindow *login = qobject_cast<LoginWindow*>(w);
            if (login) {
                login->resetFields();
            }

            w->show();
            w->raise();
            w->activateWindow();
            break;
        }
    }

    this->close();
}


