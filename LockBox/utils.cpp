#include "utils.h"
#include <QRandomGenerator>
#include <QRegularExpression>

QString Utils::generatePassword(int length) {
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+-=";
    QString password;
    for (int i = 0; i < length; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        password.append(chars.at(index));
    }
    return password;
}

int Utils::calculatePasswordStrength(const QString &password) {
    int score = 0;
    if (password.length() >= 8) score++;

    if (password.contains(QRegularExpression("[A-Z]"))) score++;
    if (password.contains(QRegularExpression("[a-z]"))) score++;
    if (password.contains(QRegularExpression("[0-9]"))) score++;
    if (password.contains(QRegularExpression("[^A-Za-z0-9]"))) score++;

    return score;
}

QString Utils::strengthLabel(int score) {
    switch (score) {
    case 0:
    case 1: return "Weak";
    case 2:
    case 3: return "Medium";
    default: return "Strong";
    }
}
