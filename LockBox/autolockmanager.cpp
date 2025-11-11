#include "autolockmanager.h"

AutoLockManager::AutoLockManager(QObject *parent, int timeoutMs)
    : QObject(parent),
    m_timeoutMs(timeoutMs)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(m_timeoutMs);
    m_timer->setSingleShot(true);

    connect(m_timer, &QTimer::timeout, this, [this]() {
        qDebug() << "Auto-lock timer triggered!";
        emit timeout();
    });

    // Install global event filter
    qApp->installEventFilter(this);
}

bool AutoLockManager::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::KeyPress:
    case QEvent::Wheel:
    case QEvent::TouchBegin:
        resetTimer();
        break;
    default:
        break;
    }
    return QObject::eventFilter(obj, event);
}

void AutoLockManager::resetTimer()
{
    if (m_timer->isActive())
        m_timer->start(m_timeoutMs); // restart countdown
}

void AutoLockManager::start()
{
    m_timer->start();
}

void AutoLockManager::stop()
{
    m_timer->stop();
}
