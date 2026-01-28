#pragma once
#include <QObject>
#include <QJsonArray>
#include <QString>

class AutofillManager : public QObject
{
    Q_OBJECT

public:
    static QJsonArray getCredentialsForUrl(const QString &url);
};
