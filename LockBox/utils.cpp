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

    // --- Length ---
    int len = password.length();
    if (len >= 8) score += 2;
    if (len >= 12) score += 2;
    if (len >= 16) score += 2;

    // --- Character variety ---
    int upper = password.count(QRegularExpression("[A-Z]"));
    int lower = password.count(QRegularExpression("[a-z]"));
    int digits = password.count(QRegularExpression("[0-9]"));
    int symbols = password.count(QRegularExpression("[^A-Za-z0-9]"));

    int variety = 0;
    if (upper > 0) variety++;
    if (lower > 0) variety++;
    if (digits > 0) variety++;
    if (symbols > 0) variety++;

    score += variety * 2; // give more weight to diversity

    // --- Penalize repeated characters ---
    QRegularExpression repeatRegex("(.)\\1{2,}");
    if (repeatRegex.match(password).hasMatch()) score -= 2;

    // --- Penalize sequential characters ---
    QString lowerPwd = password.toLower();
    for (int i = 0; i < lowerPwd.length() - 2; i++) {
        QChar c1 = lowerPwd[i], c2 = lowerPwd[i+1], c3 = lowerPwd[i+2];
        if (c2.unicode() == c1.unicode() + 1 && c3.unicode() == c2.unicode() + 1) score -= 2;
        if (c1.isDigit() && c2.isDigit() && c3.isDigit()) {
            if ((c2.unicode() == c1.unicode() + 1) && (c3.unicode() == c2.unicode() + 1)) score -= 2;
        }
    }


    if (score < 0) score = 0;
    if (score > 10) score = 10;

    return score;
}


QString Utils::strengthLabel(int score) {
    if (score <= 2) return "Very Weak";
    else if (score <= 4) return "Weak";
    else if (score <= 6) return "Medium";
    else if (score <= 8) return "Strong";
    else return "Very Strong";
}



