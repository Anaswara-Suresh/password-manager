#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QByteArray>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // ✅ Constructor must match the .cpp file
    explicit MainWindow(const QByteArray &key, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // ✅ These must match the slot names in .cpp
    void onViewPasswordsClicked();
    void on_addPasswordButton_clicked();

private:
    Ui::MainWindow *ui;
    QByteArray masterKey;   // ✅ add this to store the key
};

#endif // MAINWINDOW_H
