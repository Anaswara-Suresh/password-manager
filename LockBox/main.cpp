#include <QApplication>
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
    }

    Database::initialize();


    AddPasswordPage w;
    w.show();

    return app.exec();
}
