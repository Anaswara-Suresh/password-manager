/********************************************************************************
** Form generated from reading UI file 'addpasswordpage.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ADDPASSWORDPAGE_H
#define UI_ADDPASSWORDPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_addpasswordpage
{
public:
    QWidget *centralwidget;
    QLabel *label;
    QLabel *labelSite;
    QLabel *labelUser;
    QLabel *labelPass;
    QLineEdit *siteEdit;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *generateButton;
    QPushButton *analyzeButton;
    QPushButton *addButton;
    QLabel *strengthLabel;
    QMenuBar *menubar;

    void setupUi(QMainWindow *addpasswordpage)
    {
        if (addpasswordpage->objectName().isEmpty())
            addpasswordpage->setObjectName("addpasswordpage");
        addpasswordpage->resize(800, 600);
        centralwidget = new QWidget(addpasswordpage);
        centralwidget->setObjectName("centralwidget");
        label = new QLabel(centralwidget);
        label->setObjectName("label");
        label->setGeometry(QRect(130, 50, 161, 16));
        QFont font;
        font.setPointSize(13);
        font.setBold(true);
        font.setItalic(false);
        font.setStrikeOut(false);
        label->setFont(font);
        labelSite = new QLabel(centralwidget);
        labelSite->setObjectName("labelSite");
        labelSite->setGeometry(QRect(50, 130, 49, 16));
        labelUser = new QLabel(centralwidget);
        labelUser->setObjectName("labelUser");
        labelUser->setGeometry(QRect(50, 160, 71, 16));
        labelPass = new QLabel(centralwidget);
        labelPass->setObjectName("labelPass");
        labelPass->setGeometry(QRect(50, 190, 71, 16));
        siteEdit = new QLineEdit(centralwidget);
        siteEdit->setObjectName("siteEdit");
        siteEdit->setGeometry(QRect(150, 130, 113, 24));
        usernameEdit = new QLineEdit(centralwidget);
        usernameEdit->setObjectName("usernameEdit");
        usernameEdit->setGeometry(QRect(150, 160, 113, 24));
        passwordEdit = new QLineEdit(centralwidget);
        passwordEdit->setObjectName("passwordEdit");
        passwordEdit->setGeometry(QRect(150, 190, 113, 24));
        generateButton = new QPushButton(centralwidget);
        generateButton->setObjectName("generateButton");
        generateButton->setGeometry(QRect(280, 190, 131, 31));
        analyzeButton = new QPushButton(centralwidget);
        analyzeButton->setObjectName("analyzeButton");
        analyzeButton->setGeometry(QRect(280, 230, 131, 31));
        addButton = new QPushButton(centralwidget);
        addButton->setObjectName("addButton");
        addButton->setGeometry(QRect(150, 270, 121, 31));
        strengthLabel = new QLabel(centralwidget);
        strengthLabel->setObjectName("strengthLabel");
        strengthLabel->setGeometry(QRect(150, 230, 111, 20));
        addpasswordpage->setCentralWidget(centralwidget);
        menubar = new QMenuBar(addpasswordpage);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 24));
        addpasswordpage->setMenuBar(menubar);

        retranslateUi(addpasswordpage);

        QMetaObject::connectSlotsByName(addpasswordpage);
    } // setupUi

    void retranslateUi(QMainWindow *addpasswordpage)
    {
        addpasswordpage->setWindowTitle(QCoreApplication::translate("addpasswordpage", "MainWindow", nullptr));
        label->setText(QCoreApplication::translate("addpasswordpage", "Add New Password", nullptr));
        labelSite->setText(QCoreApplication::translate("addpasswordpage", "Site", nullptr));
        labelUser->setText(QCoreApplication::translate("addpasswordpage", "Username", nullptr));
        labelPass->setText(QCoreApplication::translate("addpasswordpage", "Password", nullptr));
        generateButton->setText(QCoreApplication::translate("addpasswordpage", "Generate Password", nullptr));
        analyzeButton->setText(QCoreApplication::translate("addpasswordpage", "Check Strength", nullptr));
        addButton->setText(QCoreApplication::translate("addpasswordpage", "Add Password", nullptr));
        strengthLabel->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class addpasswordpage: public Ui_addpasswordpage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADDPASSWORDPAGE_H
