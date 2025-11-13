#ifndef PASSWORDLIST_H
#define PASSWORDLIST_H

#include <QWidget>
#include <QPushButton>

namespace Ui {
class PasswordList;
}

class PasswordList : public QWidget
{
    Q_OBJECT

public:
    explicit PasswordList(const QByteArray &key, const QString &username, QWidget *parent = nullptr);
    ~PasswordList();

private slots:
    void loadPasswords(const QString &filter = QString());
    void onEditButtonClicked();
    void onSearchTextChanged(const QString &text);
    void onDeleteButtonClicked();
    void refreshTable();
    void onSearchClicked();
    void onCheckAllWithHIBP();
    void updateStatus(const QString &message);

private:
    Ui::PasswordList *ui;
    QByteArray masterKey;
    QString currentUsername;
};

#endif // PASSWORDLIST_H
