#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QByteArray>
#include <QString>
#include "passwordlist.h"
#include "addpasswordpage.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QByteArray &key, const QString &username, QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void passwordAdded();

private:
    Ui::MainWindow *ui;
    QByteArray masterKey;
    QString currentUser;

    QStackedWidget *stackedWidget;
    PasswordList *passwordListPage;
    AddPasswordPage *addPasswordWindow = nullptr;

private slots:
    void onViewPasswordsClicked();
    void on_addPasswordButton_clicked();
    void on_btnLogout_clicked();

};

#endif // MAINWINDOW_H
