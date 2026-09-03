#include "touchpadmanager.h"
#include "touchpadadaptor.h"

#include "input/kwininputbackend.h"

#include <QDBusConnection>

TouchpadManager::TouchpadManager(QObject *parent)
    : QObject(parent)
    , m_backend(new KWinInputBackend(KWinInputBackend::DeviceType::Touchpad, this))
{
    new TouchpadAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/com/cutefish/Services/Input/Touchpad"), this);

    connect(m_backend, &KWinInputBackend::devicesChanged, this, [this] {
        emit availableChanged();
        emit enabledChanged();
        emit tapToClickChanged();
        emit naturalScrollChanged();
        emit pointerAccelerationChanged();
    });
}

bool TouchpadManager::available() const
{
    return m_backend->available();
}

bool TouchpadManager::enabled() const
{
    return m_backend->booleanProperty(QStringLiteral("enabled"), true);
}

void TouchpadManager::setEnabled(bool enabled)
{
    if (this->enabled() == enabled)
        return;
    m_backend->setBooleanProperty(QStringLiteral("enabled"), enabled);
    emit enabledChanged();
}

bool TouchpadManager::tapToClick() const
{
    return m_backend->booleanProperty(QStringLiteral("tapToClick"));
}

void TouchpadManager::setTapToClick(bool value)
{
    if (tapToClick() == value)
        return;
    m_backend->setBooleanProperty(QStringLiteral("tapToClick"), value);
    emit tapToClickChanged();
}

bool TouchpadManager::naturalScroll() const
{
    return m_backend->booleanProperty(QStringLiteral("naturalScroll"));
}

void TouchpadManager::setNaturalScroll(bool naturalScroll)
{
    if (this->naturalScroll() == naturalScroll)
        return;
    m_backend->setBooleanProperty(QStringLiteral("naturalScroll"), naturalScroll);
    emit naturalScrollChanged();
}

qreal TouchpadManager::pointerAcceleration() const
{
    return m_backend->realProperty(QStringLiteral("pointerAcceleration"));
}

void TouchpadManager::setPointerAcceleration(qreal value)
{
    value = qBound<qreal>(-1.0, value, 1.0);
    if (qFuzzyCompare(1.0 + pointerAcceleration(), 1.0 + value))
        return;
    m_backend->setRealProperty(QStringLiteral("pointerAcceleration"), value);
    emit pointerAccelerationChanged();
}
