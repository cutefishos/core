#include "touchpadmanager.h"
#include "touchpadadaptor.h"

#include <QDebug>

TouchpadManager::TouchpadManager(QObject *parent)
    : QObject(parent)
    , m_backend(XlibBackend::initialize())
{
    // init dbus
    new TouchpadAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Touchpad"), this);

    m_backend->getConfig();
    m_backend->applyConfig();
}

bool TouchpadManager::available() const
{
    return m_backend->isTouchpadAvailable();
}

bool TouchpadManager::enabled() const
{
    return m_backend->isTouchpadEnabled();
}

void TouchpadManager::setEnabled(bool enabled)
{
    m_backend->setTouchpadEnabled(enabled);
    m_backend->applyConfig();
}

bool TouchpadManager::tapToClick() const
{
    return m_backend->tapToClick();
}

void TouchpadManager::setTapToClick(bool value)
{
    m_backend->setTapToClick(value);
    m_backend->applyConfig();
}

bool TouchpadManager::naturalScroll() const
{
    return m_backend->naturalScroll();
}

void TouchpadManager::setNaturalScroll(bool naturalScroll)
{
    m_backend->setNaturalScroll(naturalScroll);
    m_backend->applyConfig();
}

qreal TouchpadManager::pointerAcceleration() const
{
    return m_backend->pointerAcceleration();
}

void TouchpadManager::setPointerAcceleration(qreal value)
{
    qDebug() << value;
    m_backend->setPointerAcceleration(value);
    m_backend->applyConfig();
}
