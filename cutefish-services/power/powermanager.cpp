#include "powermanager.h"

#include "../battery/upowermanager.h"
#include "cpumanagement.h"
#include "dimdisplayaction.h"

namespace
{
constexpr auto s_powerGroup = "Power";

int normalizedTimeout(int timeout)
{
    return timeout < 0 ? -1 : qMax(1, timeout);
}

int readTimeout(QSettings &settings, const QString &key, const QString &legacyKey, int fallback)
{
    settings.beginGroup(QLatin1String(s_powerGroup));
    const bool hasValue = settings.contains(key);
    const int value = hasValue ? settings.value(key).toInt() : fallback;
    settings.endGroup();

    if (hasValue)
        return normalizedTimeout(value);

    if (!legacyKey.isEmpty() && settings.contains(legacyKey))
        return normalizedTimeout(settings.value(legacyKey).toInt());

    return fallback;
}

}

PowerManager::PowerManager(UPowerManager *upowerManager,
                           CPUManagement *cpuManagement,
                           QObject *parent)
    : QObject(parent)
    , m_settings(QStringLiteral("cutefishos"), QStringLiteral("power"))
    , m_upowerManager(upowerManager)
    , m_cpuManagement(cpuManagement)
    , m_dimDisplayAction(new DimDisplayAction(this))
{
    if (m_cpuManagement) {
        connect(m_cpuManagement, &CPUManagement::modeChanged,
                this, &PowerManager::modeChanged);
    }

    loadSettings();

    m_onBattery = m_upowerManager && m_upowerManager->onBattery();

    if (m_upowerManager) {
        connect(m_upowerManager, &UPowerManager::onBatteryChanged,
                this, &PowerManager::onBatteryChanged);
        connect(m_upowerManager, &UPowerManager::lidClosedChanged,
                this, &PowerManager::onLidClosedChanged);

        m_dimDisplayAction->setLidPresent(m_upowerManager->lidIsPresent());
    }

    applyPolicy();

    if (m_upowerManager && m_upowerManager->lidIsClosed())
        m_dimDisplayAction->handleLidClosed();
}

int PowerManager::mode() const
{
    return m_cpuManagement ? m_cpuManagement->mode() : -1;
}

void PowerManager::setMode(int mode)
{
    if (m_cpuManagement)
        m_cpuManagement->setMode(mode);
}

void PowerManager::loadSettings()
{
    m_batteryScreenOff = readTimeout(m_settings, QStringLiteral("BatteryScreenOff"),
                                     QStringLiteral("CloseScreenTimeout"), 300);
    m_acScreenOff = readTimeout(m_settings, QStringLiteral("ACScreenOff"),
                                QStringLiteral("CloseScreenTimeout"), 1200);
}

void PowerManager::applyPolicy()
{
    const int timeout = m_onBattery ? m_batteryScreenOff : m_acScreenOff;

    m_dimDisplayAction->setPolicy(timeout, m_onBattery ? 300 : 900);
}

void PowerManager::writePowerSetting(const QString &key, const QVariant &value)
{
    m_settings.beginGroup(QLatin1String(s_powerGroup));
    m_settings.setValue(key, value);
    m_settings.endGroup();
    m_settings.sync();
}

void PowerManager::setBatteryScreenOff(int timeout)
{
    m_batteryScreenOff = normalizedTimeout(timeout);
    writePowerSetting(QStringLiteral("BatteryScreenOff"), m_batteryScreenOff);
    applyPolicy();
}

void PowerManager::setACScreenOff(int timeout)
{
    m_acScreenOff = normalizedTimeout(timeout);
    writePowerSetting(QStringLiteral("ACScreenOff"), m_acScreenOff);
    applyPolicy();
}

void PowerManager::setDimDisplayTimeout(int timeout)
{
    setBatteryScreenOff(timeout);
    setACScreenOff(timeout);
}

void PowerManager::onBatteryChanged()
{
    m_onBattery = m_upowerManager && m_upowerManager->onBattery();
    applyPolicy();
}

void PowerManager::onLidClosedChanged()
{
    if (!m_upowerManager)
        return;

    if (m_upowerManager->lidIsClosed())
        m_dimDisplayAction->handleLidClosed();
    else
        m_dimDisplayAction->handleLidOpened();
}
