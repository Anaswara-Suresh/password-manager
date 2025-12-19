#ifndef AUTOLOCKMANAGER_H
#define AUTOLOCKMANAGER_H

#include <QObject>
#include <QTimer>
#include <QEvent>
#include <QApplication>
#include <QDebug>

class AutoLockManager : public QObject
{
    Q_OBJECT

public:
    explicit AutoLockManager(QObject *parent = nullptr, int timeoutMs = 100000000000000);
    void resetTimer();  // manually reset timer
    void start();       // start monitoring
    void stop();        // stop monitoring

signals:
    void timeout();     // emitted when user inactive

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QTimer *m_timer;
    int m_timeoutMs;
};

#endif // AUTOLOCKMANAGER_H
