#include "totpdialog.h"
#include "totp.h"

#include <QVBoxLayout>
#include <QDateTime>
#include <QClipboard>
#include <QApplication>

TotpDialog::TotpDialog(const QString &secret, QWidget *parent)
    : QDialog(parent), totpSecret(secret)
{
    setWindowTitle("One-Time Password");
    setFixedSize(300, 180);

    otpLabel = new QLabel(this);
    timerLabel = new QLabel(this);
    copyButton = new QPushButton("Copy", this);

    QFont f;
    f.setPointSize(24);
    f.setBold(true);
    otpLabel->setFont(f);
    otpLabel->setAlignment(Qt::AlignCenter);
    timerLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(otpLabel);
    layout->addWidget(timerLabel);
    layout->addWidget(copyButton);

    connect(copyButton, &QPushButton::clicked,
            this, &TotpDialog::copyTotp);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout,
            this, &TotpDialog::updateTotp);
    timer->start(1000);

    updateTotp();
}

void TotpDialog::updateTotp()
{
    QString code = TOTP::generate(totpSecret);
    otpLabel->setText(code);

    int period = 30;
    int remaining =
        period - (QDateTime::currentSecsSinceEpoch() % period);

    timerLabel->setText(
        QString("Valid for %1 seconds").arg(remaining));
}

void TotpDialog::copyTotp()
{
    QApplication::clipboard()->setText(otpLabel->text());
}
