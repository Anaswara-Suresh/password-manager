/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QSpacerItem *topSpacer;
    QLabel *logoLabel;
    QSpacerItem *bottomSpacer;
    QLabel *titleLabel;
    QPushButton *viewPasswordsButton;
    QPushButton *addPasswordButton;
    QPushButton *btnLogout;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        topSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(topSpacer);

        logoLabel = new QLabel(centralwidget);
        logoLabel->setObjectName("logoLabel");
        logoLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        logoLabel->setPixmap(QPixmap(QString::fromUtf8("LockBox_Logo.png")));
        logoLabel->setScaledContents(true);
        logoLabel->setMinimumSize(QSize(200, 200));
        logoLabel->setMaximumSize(QSize(200, 200));

        verticalLayout->addWidget(logoLabel, 0, Qt::AlignHCenter);

        bottomSpacer = new QSpacerItem(20, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(bottomSpacer);

        titleLabel = new QLabel(centralwidget);
        titleLabel->setObjectName("titleLabel");
        QFont font;
        font.setPointSize(14);
        font.setBold(true);
        titleLabel->setFont(font);
        titleLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);

        viewPasswordsButton = new QPushButton(centralwidget);
        viewPasswordsButton->setObjectName("viewPasswordsButton");
        viewPasswordsButton->setMinimumSize(QSize(200, 40));

        verticalLayout->addWidget(viewPasswordsButton, 0, Qt::AlignHCenter);

        addPasswordButton = new QPushButton(centralwidget);
        addPasswordButton->setObjectName("addPasswordButton");
        addPasswordButton->setMinimumSize(QSize(200, 40));

        verticalLayout->addWidget(addPasswordButton, 0, Qt::AlignHCenter);

        btnLogout = new QPushButton(centralwidget);
        btnLogout->setObjectName("btnLogout");
        btnLogout->setMinimumSize(QSize(200, 40));

        verticalLayout->addWidget(btnLogout, 0, Qt::AlignHCenter);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Password Manager", nullptr));
        titleLabel->setText(QCoreApplication::translate("MainWindow", "Welcome to Your Password Manager", nullptr));
        viewPasswordsButton->setText(QCoreApplication::translate("MainWindow", "View Saved Passwords", nullptr));
        addPasswordButton->setText(QCoreApplication::translate("MainWindow", "Add New Password", nullptr));
        btnLogout->setText(QCoreApplication::translate("MainWindow", "Logout", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
