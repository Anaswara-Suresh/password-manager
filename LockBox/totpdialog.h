#ifndef TOTPDIALOG_H
#define TOTPDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

class TotpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TotpDialog(const QString &secret, QWidget *parent = nullptr);

private slots:
    void updateTotp();
    void copyTotp();

private:
    QString totpSecret;
    QLabel *otpLabel;
    QLabel *timerLabel;
    QPushButton *copyButton;
    QTimer *timer;
};

#endif
