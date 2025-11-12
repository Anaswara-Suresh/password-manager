/********************************************************************************
** Form generated from reading UI file 'passwordlist.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PASSWORDLIST_H
#define UI_PASSWORDLIST_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PasswordList
{
public:
    QTableWidget *tableWidget;

    void setupUi(QWidget *PasswordList)
    {
        if (PasswordList->objectName().isEmpty())
            PasswordList->setObjectName("PasswordList");
        PasswordList->resize(900, 600);
        tableWidget = new QTableWidget(PasswordList);
        tableWidget->setObjectName("tableWidget");
        tableWidget->setGeometry(QRect(20, 20, 860, 550));

        retranslateUi(PasswordList);

        QMetaObject::connectSlotsByName(PasswordList);
    } // setupUi

    void retranslateUi(QWidget *PasswordList)
    {
        PasswordList->setWindowTitle(QCoreApplication::translate("PasswordList", "Saved Passwords", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PasswordList: public Ui_PasswordList {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PASSWORDLIST_H
