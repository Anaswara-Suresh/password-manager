#include <QApplication>
#include "loginwindow.h"
#include "addpasswordpage.h"
#include "database.h"
#include "crypto.h"
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    if (!Crypto::initialize()) {
        qCritical() << "FATAL: Could not initialize cryptography library. Exiting.";
        return 1;
    }

    QFile styleFile("style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        app.setStyleSheet(style);
        styleFile.close();
    } else {
        qWarning() << "Could not open style.qss";
    }


    if (!Database::initialize()) {
        qCritical() << "FATAL: Could not initialize database. Exiting.";
        return 1;
    }


    LoginWindow loginWindow;
    loginWindow.show();

    return app.exec();
}