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
    explicit PasswordList(QWidget *parent = nullptr);
    ~PasswordList();

private slots:
    void loadPasswords();
    void onEditButtonClicked();    // ✅ no int param — uses sender()->property("entryId")
    void onDeleteButtonClicked();  // ✅ same
    void refreshTable();

private:
    Ui::PasswordList *ui;
};

#endif // PASSWORDLIST_H
