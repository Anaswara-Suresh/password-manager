#ifndef ADDPASSWORDPAGE_H
#define ADDPASSWORDPAGE_H
#include <QMainWindow>

namespace Ui {
class addpasswordpage;
}


class AddPasswordPage : public QMainWindow
{
    Q_OBJECT

public:

    explicit AddPasswordPage(QWidget *parent = nullptr);
    ~AddPasswordPage();

private slots:
    void on_generateButton_clicked();
    void on_analyzeButton_clicked();
    void on_addButton_clicked();

private:
    Ui::addpasswordpage *ui;
};

#endif
