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
    explicit PasswordList(const QByteArray &key, QWidget *parent = nullptr);
    QByteArray masterKey;

    ~PasswordList();

private slots:
    void loadPasswords();
    void onEditButtonClicked();    
    void onDeleteButtonClicked();  
    void refreshTable();

private:
    Ui::PasswordList *ui;
};

#endif 
