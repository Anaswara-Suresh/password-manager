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
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PasswordList
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *searchLayout;
    QLineEdit *searchEdit;
    QPushButton *searchButton;
    QPushButton *checkAllButton;
    QTableWidget *tableWidget;
    QLabel *statusLabel;

    void setupUi(QWidget *PasswordList)
    {
        if (PasswordList->objectName().isEmpty())
            PasswordList->setObjectName("PasswordList");
        PasswordList->resize(900, 600);
        verticalLayout = new QVBoxLayout(PasswordList);
        verticalLayout->setSpacing(10);
        verticalLayout->setObjectName("verticalLayout");
        searchLayout = new QHBoxLayout();
        searchLayout->setObjectName("searchLayout");
        searchEdit = new QLineEdit(PasswordList);
        searchEdit->setObjectName("searchEdit");

        searchLayout->addWidget(searchEdit);

        searchButton = new QPushButton(PasswordList);
        searchButton->setObjectName("searchButton");

        searchLayout->addWidget(searchButton);

        checkAllButton = new QPushButton(PasswordList);
        checkAllButton->setObjectName("checkAllButton");

        searchLayout->addWidget(checkAllButton);


        verticalLayout->addLayout(searchLayout);

        tableWidget = new QTableWidget(PasswordList);
        tableWidget->setObjectName("tableWidget");
        tableWidget->setAlternatingRowColors(true);
        tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidget->setShowGrid(true);
        tableWidget->setGridStyle(Qt::SolidLine);
        tableWidget->setSortingEnabled(true);
        tableWidget->setStyleSheet(QString::fromUtf8("\n"
"       QHeaderView::section {\n"
"           background-color: #1f1f1f;\n"
"           color: white;\n"
"           padding: 6px;\n"
"           border: none;\n"
"           font-weight: bold;\n"
"       }\n"
"       QTableWidget {\n"
"           gridline-color: #333;\n"
"           background-color: #121212;\n"
"           color: #e0e0e0;\n"
"           alternate-background-color: #1e1e1e;\n"
"           selection-background-color: #00b894;\n"
"           selection-color: #ffffff;\n"
"           border: 1px solid #333;\n"
"           border-radius: 8px;\n"
"       }\n"
"       QScrollBar:vertical {\n"
"           background: #1f1f1f;\n"
"           width: 12px;\n"
"           margin: 0px;\n"
"       }\n"
"       QScrollBar::handle:vertical {\n"
"           background: #444;\n"
"           border-radius: 6px;\n"
"       }\n"
"       QScrollBar::handle:vertical:hover {\n"
"           background: #00b894;\n"
"       }\n"
"      "));

        verticalLayout->addWidget(tableWidget);

        statusLabel = new QLabel(PasswordList);
        statusLabel->setObjectName("statusLabel");
        statusLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        statusLabel->setStyleSheet(QString::fromUtf8("\n"
"       QLabel {\n"
"           background-color: #202020;\n"
"           color: #00b894;\n"
"           border-radius: 6px;\n"
"           padding: 6px 10px;\n"
"           margin-top: 5px;\n"
"       }\n"
"      "));

        verticalLayout->addWidget(statusLabel);


        retranslateUi(PasswordList);

        QMetaObject::connectSlotsByName(PasswordList);
    } // setupUi

    void retranslateUi(QWidget *PasswordList)
    {
        PasswordList->setWindowTitle(QCoreApplication::translate("PasswordList", "Saved Passwords", nullptr));
        searchEdit->setPlaceholderText(QCoreApplication::translate("PasswordList", "\360\237\224\215 Search by site...", nullptr));
        searchButton->setText(QCoreApplication::translate("PasswordList", "Search", nullptr));
        checkAllButton->setText(QCoreApplication::translate("PasswordList", "Check All with HIBP", nullptr));
        statusLabel->setText(QCoreApplication::translate("PasswordList", "Ready", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PasswordList: public Ui_PasswordList {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PASSWORDLIST_H
