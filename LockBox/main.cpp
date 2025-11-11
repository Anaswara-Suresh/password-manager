#include <QApplication>
#include "loginwindow.h"
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

    if (!Database::initialize()) {
        qCritical() << "FATAL: Could not initialize database. Exiting.";
        return 1;
    }

    QFile styleFile("style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        app.setStyleSheet(QLatin1String(styleFile.readAll()));
        styleFile.close();
    }

    LoginWindow login;
    login.show();

    return app.exec();
}
