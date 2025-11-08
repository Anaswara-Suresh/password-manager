#ifndef UTILS_H
#define UTILS_H

#include <QString>

namespace Utils {

QString generatePassword(int length = 12);
int calculatePasswordStrength(const QString &password);
QString strengthLabel(int score);

}

#endif
