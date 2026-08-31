#include "powermanager.h"

#include "../battery/upowermanager.h"
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

bool readBool(QSettings &settings, const QString &key, const QString &legacyKey, bool fallback)
{
    settings.beginGroup(QLatin1String(s_powerGroup));
    const bool hasValue = settings.contains(key);
    const bool value = hasValue ? settings.value(key).toBool() : fallback;
    settings.endGroup();

    if (hasValue)
        return value;

    return settings.value(legacyKey, fallback).toBool();
}
}

PowerManager::PowerManager(UPowerManager *upowerManager, QObject *parent)
    : QObject(parent)
    , m_settings(QStringLiteral("cutefishos"), QStringLiteral("power"))
    , m_upowerManager(upowerManager)
    , m_dimDisplayAction(new DimDisplayAction(this))
{
    loadSettings();

    m_onBattery = m_upowerManager && m_upowerManager->onBattery();

    if (m_upowerManager) {
        connect(m_upowerManager, &UPowerManager::onBatteryChanged,
                this, &PowerManager::onBatteryChanged);
    }

    applyPolicy();
}

void PowerManager::loadSettings()
{
    m_batteryScreenOff = readTimeout(m_settings, QStringLiteral("BatteryScreenOff"),
                                     QStringLiteral("CloseScreenTimeout"), 300);
    m_acScreenOff = readTimeout(m_settings, QStringLiteral("ACScreenOff"),
                                QStringLiteral("CloseScreenTimeout"), 1200);
    m_sleepWhenClosedScreen = readBool(m_settings, QStringLiteral("SleepWhenClosedScreen"),
                                       QStringLiteral("SleepWhenClosedScreen"), false);
    m_lockWhenClosedScreen = readBool(m_settings, QStringLiteral("LockWhenClosedScreen"),
                                      QStringLiteral("LockWhenClosedScreen"), true);
}

void PowerManager::applyPolicy()
{
    const int timeout = m_onBattery ? m_batteryScreenOff : m_acScreenOff;

    m_dimDisplayAction->setTimeout(timeout);
    m_dimDisplayAction->setSleep(m_sleepWhenClosedScreen);
    m_dimDisplayAction->setLock(m_lockWhenClosedScreen);
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

void PowerManager::setSleepWhenClosedScreen(bool enabled)
{
    m_sleepWhenClosedScreen = enabled;
    writePowerSetting(QStringLiteral("SleepWhenClosedScreen"), enabled);
    applyPolicy();
}

void PowerManager::setLockWhenClosedScreen(bool enabled)
{
    m_lockWhenClosedScreen = enabled;
    writePowerSetting(QStringLiteral("LockWhenClosedScreen"), enabled);
    applyPolicy();
}

void PowerManager::onBatteryChanged()
{
    m_onBattery = m_upowerManager && m_upowerManager->onBattery();
    applyPolicy();
}
