#ifndef ADDPASSWORDPAGE_H
#define ADDPASSWORDPAGE_H

#include <QMainWindow>
#include <QByteArray>
#include <QString>

namespace Ui {
class addpasswordpage;
}

class AddPasswordPage : public QMainWindow
{
    Q_OBJECT

public:
    explicit AddPasswordPage(QWidget *parent = nullptr,
                             const QByteArray &derivedKey = QByteArray(),
                             const QString &currentUsername = QString());
    ~AddPasswordPage();

private slots:
    void on_generateButton_clicked();
    void on_analyzeButton_clicked();
    void on_addButton_clicked();

private:
    Ui::addpasswordpage *ui;
    QByteArray m_derivedKey;
    QString m_currentUsername;
};

#endif
