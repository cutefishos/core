#include "mousemanager.h"
#include "mouseadaptor.h"

#include "input/kwininputbackend.h"

#include <QDBusConnection>

Mouse::Mouse(QObject *parent)
    : QObject(parent)
    , m_backend(new KWinInputBackend(KWinInputBackend::DeviceType::Pointer, this))
{
    new MouseAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Mouse"), this);

    connect(m_backend, &KWinInputBackend::devicesChanged, this, [this] {
        emit availableChanged();
        emit leftHandedChanged();
        emit accelerationChanged();
        emit naturalScrollChanged();
        emit pointerAccelerationChanged();
    });
}

Mouse::~Mouse() = default;

bool Mouse::available() const
{
    return m_backend->available();
}

bool Mouse::leftHanded() const
{
    return m_backend->booleanProperty(QStringLiteral("leftHanded"));
}

void Mouse::setLeftHanded(bool enabled)
{
    if (leftHanded() == enabled)
        return;
    m_backend->setBooleanProperty(QStringLiteral("leftHanded"), enabled);
    emit leftHandedChanged();
}

bool Mouse::acceleration() const
{
    return m_backend->booleanProperty(QStringLiteral("pointerAccelerationProfileFlat"));
}

void Mouse::setAcceleration(bool enabled)
{
    if (acceleration() == enabled)
        return;
    m_backend->setBooleanProperty(QStringLiteral("pointerAccelerationProfileFlat"), enabled);
    m_backend->setBooleanProperty(QStringLiteral("pointerAccelerationProfileAdaptive"), !enabled);
    emit accelerationChanged();
}

bool Mouse::naturalScroll() const
{
    return m_backend->booleanProperty(QStringLiteral("naturalScroll"));
}

void Mouse::setNaturalScroll(bool enabled)
{
    if (naturalScroll() == enabled)
        return;
    m_backend->setBooleanProperty(QStringLiteral("naturalScroll"), enabled);
    emit naturalScrollChanged();
}

qreal Mouse::pointerAcceleration() const
{
    return m_backend->realProperty(QStringLiteral("pointerAcceleration"));
}

void Mouse::setPointerAcceleration(qreal value)
{
    value = qBound<qreal>(-1.0, value, 1.0);
    if (qFuzzyCompare(1.0 + pointerAcceleration(), 1.0 + value))
        return;
    m_backend->setRealProperty(QStringLiteral("pointerAcceleration"), value);
    emit pointerAccelerationChanged();
}
