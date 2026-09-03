#include "mousemanager.h"
#include "mouseadaptor.h"

#include "input/kwininputbackend.h"

#include <QDBusConnection>

namespace
{
constexpr auto s_leftHandedKey = "leftHanded";
constexpr auto s_naturalScrollKey = "naturalScroll";
constexpr auto s_pointerAccelerationKey = "pointerAcceleration";
constexpr auto s_accelerationProfileKey = "accelerationProfile";
}

Mouse::Mouse(QObject *parent)
    : QObject(parent)
    , m_backend(new KWinInputBackend(KWinInputBackend::DeviceType::Pointer, this))
    , m_settings(QSettings::UserScope, QStringLiteral("cutefishos"), QStringLiteral("mouse"))
{
    new MouseAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/com/cutefish/Services/Input/Mouse"), this);

    connect(m_backend, &KWinInputBackend::devicesChanged, this, [this] {
        restoreSettings();
        emit availableChanged();
        emit leftHandedChanged();
        emit accelerationChanged();
        emit naturalScrollChanged();
        emit pointerAccelerationChanged();
    });
}

Mouse::~Mouse() = default;

void Mouse::restoreSettings()
{
    if (!m_backend->available())
        return;

    const auto restoreBoolean = [this](const QString &property, const char *key) {
        if (m_settings.contains(QLatin1String(key)))
            m_backend->setBooleanProperty(property, m_settings.value(QLatin1String(key)).toBool());
    };

    restoreBoolean(QStringLiteral("leftHanded"), s_leftHandedKey);
    restoreBoolean(QStringLiteral("naturalScroll"), s_naturalScrollKey);
    restoreBoolean(QStringLiteral("pointerAccelerationProfileFlat"), s_accelerationProfileKey);

    if (m_settings.contains(QLatin1String(s_pointerAccelerationKey))) {
        m_backend->setRealProperty(QStringLiteral("pointerAcceleration"),
                                   m_settings.value(QLatin1String(s_pointerAccelerationKey)).toReal());
    }
}

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
    m_settings.setValue(QLatin1String(s_leftHandedKey), enabled);
    m_settings.sync();
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
    m_settings.setValue(QLatin1String(s_accelerationProfileKey), enabled);
    m_settings.sync();
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
    m_settings.setValue(QLatin1String(s_naturalScrollKey), enabled);
    m_settings.sync();
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
    m_settings.setValue(QLatin1String(s_pointerAccelerationKey), value);
    m_settings.sync();
    m_backend->setRealProperty(QStringLiteral("pointerAcceleration"), value);
    emit pointerAccelerationChanged();
}
