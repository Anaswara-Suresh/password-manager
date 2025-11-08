#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QByteArray>

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void handleLogin(); // Slot to handle login click

private:
    Ui::LoginWindow *ui;
    QByteArray derivedKey;
};

#endif // LOGINWINDOW_H
